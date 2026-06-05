Currently, test_oracle is faking everything, and needs to be fixed

to improve things, we need separate functions called trace*iter*<type> in the c++ version, which rather than returning a final color of a pixel, will return a vector of all the intermediate points the function visited. We'll need a vector<double> version for a future UI feature, and a vector<float> version for comparison with the wgsl port.

we'll also need a version for wgsl, but I'm thinking
