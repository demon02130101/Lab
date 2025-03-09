import numba as nb
import numpy as np
import json

def getWeightAndScale():
    weightsDict = {}
    shapeDict = {"conv1.conv": [6, 1, 5, 5],
                 "conv3.conv": [16, 6, 5, 5],
                 "conv5.conv": [120, 16, 5, 5],
                 "fc6.fc": [84, 120],
                 "output.fc": [10, 84]
                 }

    for key in shapeDict:
        Arr = np.loadtxt('./weights/'+key+".weight.csv",
                         delimiter=',').astype(int)
        shape = shapeDict[key]
        Arr = Arr.reshape(([i for i in shape]))
        weightsDict[key] = Arr

    weightsDict["outputBias"] = np.loadtxt(
        './weights/'+key+".bias.csv", delimiter=',').reshape(([1, 10])).astype(float)

    scalesDict = {}
    with open('fixed_scale.json') as json_file:
        scalesDict = json.load(json_file)
    for i in scalesDict:
      scalesDict[i] = np.array([scalesDict[i]])

    return weightsDict, scalesDict

@nb.jit()
def MaxPool2d(x, kernel_size=2, stride=2):
    N, C, H, W = x.shape
    x_out = np.zeros((N, C, int(((H-kernel_size)/stride)+1),
                     int((W-kernel_size)/stride + 1)), dtype='int32')
    # TODO
    for n in range(N):
        for c in range(C):
            for h in range(0, H - kernel_size + 1, stride):
                for w in range(0, W - kernel_size + 1, stride):
                    x_out[n, c, h // stride, w // stride] = np.max(
                        x[n, c, h:h + kernel_size, w:w + kernel_size])

    return x_out


@nb.jit()
def ReLU(x):
    # TODO
    x[x < 0] = 0
    return x


@nb.jit()
def Linear(psum_range, x, weights, weightsBias=None, psum_record=False):
  psum_record_list = [np.complex64(x) for x in range(0)]
  H, W = x.shape
  C = weights.shape[0]
  x_out = np.zeros((H, C))
  # TODO
  for h in range(H):
    for c in range(C):
        psum = 0
        for w in range(W):
            psum += x[h, w] * weights[c, w]
        if weightsBias is not None:
            psum += weightsBias[0, c]
        psum = np.maximum(np.minimum(psum, psum_range[1]), psum_range[0])
        x_out[h, c] = np.int32(psum)
        psum_record_list.append(psum)
  
  return x_out, psum_record_list


@nb.jit()
def Conv2d(psum_range, x, weights, out_channels, kernel_size=5, stride=1, bias=False, psum_record=False):
    psum_record_list = [np.complex64(x) for x in range(0)]
    N, C, H, W = x.shape
    x_out = np.zeros((N, out_channels, int(((H-kernel_size)/stride)+1),
                     int((W-kernel_size)/stride + 1)))
    # TODO
    for n in range(N):
        for c_out in range(out_channels):
            for h in range(0, H - kernel_size + 1, stride):
                for w in range(0, W - kernel_size + 1, stride):
                    psum = 0
                    for c_in in range(C):
                        for i in range(kernel_size):
                            for j in range(kernel_size):
                                psum += x[n, c_in, h + i, w + j] * weights[c_out, c_in, i, j]
                    if bias:
                        psum += 0  # Bias can be added if needed
                    psum = np.maximum(np.minimum(psum, psum_range[1]), psum_range[0])
                    x_out[n, c_out, h // stride, w // stride] = np.int32(psum)
                    psum_record_list.append(psum)

    return x_out, psum_record_list


def ActQuant(x, scale, shiftbits=16):
    # TODO
    x = np.round(x * scale) >> shiftbits
    return x


class LeNet:

    def __init__(self, psum_range_dict):
        self.psum_range = psum_range_dict
        self.weightsDict, self.scalesDict = getWeightAndScale()
        self.psum_record_dict = {}

    def forward(self, x, psum_record=False):
      # TODO
      # You should get the record of partial sums by `x, self.psum_record_dict['c1'] = Conv2d(...)`.
      # Following the sequence of the model:
      # (1) quantize input -> (2) conv1 -> (3) maxpool2 -> (4) conv3 -> (5) maxpool4 -> (6) conv5 -> (7) fc6 -> (8) output
      # Make sure to add bias in the last layer and correctly expand the bias vector to match the dimensions of the output layer for each batch of input samples.

      # Quantize the input
      x = ActQuant(x, self.scalesDict['conv1.conv'])

      # Conv1 Layer
      x, self.psum_record_dict['conv1'] = Conv2d(self.psum_range['conv1'], x, self.weightsDict['conv1.conv'], 6)
      x = ReLU(x)

      # MaxPool2 Layer
      x = MaxPool2d(x)

      # Conv3 Layer
      x, self.psum_record_dict['conv3'] = Conv2d(self.psum_range['conv3'], x, self.weightsDict['conv3.conv'], 16)
      x = ReLU(x)

      # MaxPool4 Layer
      x = MaxPool2d(x)

      # Conv5 Layer
      x, self.psum_record_dict['conv5'] = Conv2d(self.psum_range['conv5'], x, self.weightsDict['conv5.conv'], 120)
      x = ReLU(x)

      # Flatten before fully connected layers
      x = x.flatten()

      # FC6 Layer
      x, self.psum_record_dict['fc6'] = Linear(self.psum_range['fc6'], x, self.weightsDict['fc6.fc'], self.weightsDict['outputBias'])
      x = ReLU(x)

      # Output Layer
      x, self.psum_record_dict['output'] = Linear(self.psum_range['output'], x, self.weightsDict['output.fc'])
      
      return x