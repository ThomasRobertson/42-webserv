#include <iostream>
#include <iomanip>
#include <fstream>

std::string hexStr(const uint8_t data)
{
     std::stringstream ss;
     ss << std::hex;

    //  for( int i(0) ; i < len; ++i )
         ss << std::setw(2) << std::setfill('0') << (int)data;

     return ss.str();
}

int main() {
    const std::string filename = "test2.png";

    std::ifstream inputFile(filename, std::ios::binary);
    std::ofstream outputFile("test2.txt");
    std::string hexByte;

    if (inputFile.is_open()) {
        char firstByte;

        
        while (inputFile.read(&firstByte, 1))
        {
            hexByte = hexStr(static_cast<int>(firstByte));
            outputFile.write(hexByte.c_str(), hexByte.size());
            outputFile.write(" ", 1);
            std::cout << "The first byte of the PNG file is: " << hexStr(static_cast<int>(firstByte)) << std::dec << std::endl;
        }


        // std::cout << "The first byte of the PNG file is: 0x" << std::hex << static_cast<int>(firstByte) << std::dec << std::endl;
        // inputFile.read(&second, 1);
        // std::cout << "The first byte of the PNG file is: 0x" << std::hex << static_cast<int>(second) << std::dec << std::endl;
        // std::cout << "The first byte of the PNG file is: " << hexStr(static_cast<int>(second)) << std::dec << std::endl;
        inputFile.close();
    } else {
        std::cerr << "Failed to open the PNG file." << std::endl;
    }

    return 0;
}