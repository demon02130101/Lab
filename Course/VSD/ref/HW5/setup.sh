# source licsne file of CAD tool
source /users/cad/xilinx/Vivado/2022.1/settings64.csh
source /usr/cad/mentor/CIC/modelsim.cshrc
source /usr/cad/synopsys/CIC/verdi.cshrc
source /usr/cad/synopsys/CIC/synthesis.cshrc
source /usr/cad/synopsys/CIC/spyglass.cshrc

# Change locale
setenv LC_ALL "en_US.UTF-8"

# Set environment variable for ESP
setenv PATH ${PATH}:"/usr/cad/mentor/modelsim/2020.4/modeltech/bin"
setenv AMS_MODEL_TECH "/usr/cad/mentor/modelsim/2020.4/modeltech"
setenv RISCV "/users/student/mr110/CS5120/ESP_toolchain/riscv"
setenv PATH ${PATH}:"/users/student/mr110/CS5120/ESP_toolchain/riscv/bin"
setenv PATH ${PATH}:"/users/student/mr110/CS5120/ESP_toolchain/riscv32imc/bin"
setenv PATH ${PATH}:"/users/student/mr110/CS5120/ESP_toolchain/leon/bin"
setenv PATH ${PATH}:"/users/student/mr110/CS5120/ESP_toolchain/leon/mklinuximg"
setenv PATH ${PATH}:"/users/student/mr110/CS5120/ESP_toolchain/leon/sparc-elf/bin"

setenv LD_LIBRARY_PATH "/lib64/":$LD_LIBRARY_PATH