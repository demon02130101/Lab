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
    P = int(((H-kernel_size)/stride)+1) # out_H
    Q = int((W-kernel_size)/stride + 1) # out_W
    x_out = np.zeros((N, C, int(((H-kernel_size)/stride)+1),
                     int((W-kernel_size)/stride + 1)), dtype='int32')
    # TODO
    for n in range(N):
        for c in range(C):
            for p in range(P):
                for q in range(Q):
                    # Find the starting indices for the kernel window
                    h_start = p * stride
                    w_start = q * stride
                    h_end = h_start + kernel_size
                    w_end = w_start + kernel_size
                    
                    # Extract the window from the input
                    window = x[n, c, h_start:h_end, w_start:w_end]
                    
                    # Apply max pooling and assign to output
                    x_out[n][c][p][q] = np.max(window)

    return x_out 


@nb.jit()
def ReLU(x):
    # TODO
    return np.maximum(0, x)


@nb.jit()
def Linear(psum_range, x, weights, weightsBias=None, psum_record=False):
    psum_record_list = [np.complex64(x) for x in range(0)]
    H, W = x.shape
    C = weights.shape[0]
    x_out = np.zeros((H, C))

    # TODO
    for h in range(H):
        for c in range(C):
            x_out[h][c] = 0
            for w in range(W):
                x_out[h][c] += x[h][w] * weights[c][w]
                # clamp the partial sum to the specified range
                if x_out[h][c] < psum_range[0]:
                    x_out[h][c] = psum_range[0]
                elif x_out[h][c] > psum_range[1]:
                    x_out[h][c] = psum_range[1]

                if psum_record:
                    psum_record_list.append(x_out[h][c])
                        

    if weightsBias is not None:
       x_out += weightsBias

  
    return x_out, psum_record_list


@nb.jit()
def Conv2d(psum_range, x, weights, out_channels, kernel_size=5, stride=1, bias=False, psum_record=False):
    psum_record_list = [np.complex64(x) for x in range(0)]
    N, C, H, W = x.shape
    P = int(((H-kernel_size)/stride)+1) # out_H
    Q = int((W-kernel_size)/stride + 1) # out_W
    x_out = np.zeros((N, out_channels, P , Q))

    # TODO

    for n in range(N):
        for m in range(out_channels):
            for p in range(P):
                for q in range(Q):
                    x_out[n][m][p][q] = 0
                    for r in range(kernel_size):
                        for s in range(kernel_size):
                            for c in range(C):
                                h = p * stride + r
                                w = q * stride + s
                                x_out[n][m][p][q] += x[n][c][h][w] * weights[m][c][r][s]
                                
                                if(x_out[n][m][p][q] < psum_range[0]):
                                    x_out[n][m][p][q] = psum_range[0]
                                elif(x_out[n][m][p][q] > psum_range[1]):
                                    x_out[n][m][p][q] = psum_range[1]
                                
                                if psum_record:
                                    psum_record_list.append(x_out[n][m][p][q])

    return x_out, psum_record_list


def ActQuant(x, scale, shiftbits=16):
    # TODO
    x = np.round(x * scale).astype(np.int32)  # Cast to integer type before shifting
    x = x >> shiftbits  # Right shift
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

        # Quantize input
        # x = ActQuant(x, self.scalesDict['quant'])
        x = x * self.scalesDict['quant']
        x = np.clip(x, -128, 127)

        # Conv1
        x, self.psum_record_dict['c1'] = Conv2d(self.psum_range['c1'], x, self.weightsDict['conv1.conv'], out_channels=6, kernel_size=5, stride=1, psum_record=psum_record)
        x = ActQuant(x, self.scalesDict['conv1.conv'])
        x = ReLU(x)
        
        # MaxPool2
        x = MaxPool2d(x, kernel_size=2, stride=2)
        
        # Conv3
        x, self.psum_record_dict['c3'] = Conv2d(self.psum_range['c3'], x, self.weightsDict['conv3.conv'], out_channels=16, kernel_size=5, stride=1, psum_record=psum_record)
        x = ActQuant(x, self.scalesDict['conv3.conv'])
        x = ReLU(x)
        
        # MaxPool4
        x = MaxPool2d(x, kernel_size=2, stride=2)
        
        # Conv5
        x, self.psum_record_dict['c5'] = Conv2d(self.psum_range['c5'], x, self.weightsDict['conv5.conv'], out_channels=120, kernel_size=5, stride=1, psum_record=psum_record)
        x = ActQuant(x, self.scalesDict['conv5.conv'])
        x = ReLU(x)

        # Flatten
        x = x.reshape(x.shape[0], -1)  

        # FC6
        x, self.psum_record_dict['f6'] = Linear(self.psum_range['f6'], x, self.weightsDict['fc6.fc'], psum_record=psum_record)
        x = ActQuant(x, self.scalesDict['fc6.fc'])
        x = ReLU(x)
        
        # Output
        x, self.psum_record_dict['output'] = Linear(self.psum_range['output'], x, self.weightsDict['output.fc'], weightsBias=self.weightsDict['outputBias'], psum_record=psum_record)
        x = ActQuant(x, self.scalesDict['output.fc'])
        
        return x