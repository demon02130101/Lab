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
    
    return x_out


@nb.jit()
def ReLU(x):
    # TODO
    
    return x


@nb.jit()
def Linear(psum_range, x, weights, weightsBias=None, psum_record=False):
  psum_record_list = [np.complex64(x) for x in range(0)]
  H, W = x.shape
  C = weights.shape[0]
  x_out = np.zeros((H, C))
  # TODO

  return x_out, psum_record_list


@nb.jit()
def Conv2d(psum_range, x, weights, out_channels, kernel_size=5, stride=1, bias=False, psum_record=False):
    psum_record_list = [np.complex64(x) for x in range(0)]
    N, C, H, W = x.shape
    x_out = np.zeros((N, out_channels, int(((H-kernel_size)/stride)+1),
                     int((W-kernel_size)/stride + 1)))
    # TODO
    

    return x_out, psum_record_list


def ActQuant(x, scale, shiftbits=16):
    # TODO

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

      
      return x