#include <iostream>
#include <BL/init.hpp>
int main(){
    BL::setWindowInit(800,600);
    if(!BL::initWindow("Boundless",false,false)){
        return -1;
    }

    BL::terminateWindow();
    return 0;
} 