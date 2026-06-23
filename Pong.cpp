#include <iostream>
#include <array>
#include <chrono>
#include <ctime>
#include <thread>
#include <cmath>

void iniciarArray(std::array<std::array<std::string, 20>,15> &arr){
    for(int i = 0; i < 15; i++){
        for(int j = 0; j < 20; j++){
            //if(((j==0)||(j==19))&&((i>4)&&(i < 9))) arr[i][j] = " # ";
            //else 
            arr[i][j] = " - ";
        } 
    }
}

std::string buffear(std::array<std::array<std::string, 20>,15> &arr){
    std::string cadena = "";
    for(int i = 0; i < 15; i++){
        for(int j = 0; j < 20; j++) cadena+=arr[i][j];
        cadena+="\n";
    }
    return cadena;
}


int main(){
    std::array<std::array<std::string, 20>,15> arr;
    std::string buff;
    std::array<double, 2> pelota = {8, 10};
    std::array<double, 3> velocidad= {2, 0, 0};
    double teta = M_PI/3;
    
    velocidad[1] = velocidad[0]*sin(teta);
    velocidad[2] = velocidad[0]*cos(teta);

    iniciarArray(arr);
    buff = buffear(arr);

    const int tiempo = 100;

    std::chrono::steady_clock::time_point inicio, actual;
    std::chrono::duration<double, std::milli> lapso;
    inicio = std::chrono::steady_clock::now();

    arr[pelota[0]][pelota[1]] = " O ";

    while(true){
        arr[int(pelota[0])][int(pelota[1])] = "   ";
        actual = std::chrono::steady_clock::now();
        lapso = actual-inicio;
        if(lapso.count()>tiempo){
            inicio += std::chrono::milliseconds(tiempo);
            if(velocidad[1]>0) if(pelota[0] >= 0) pelota[0]-=velocidad[1];
            if(velocidad[1]<0) if(pelota[0] <= 14) pelota[0]-=velocidad[1];
            if(velocidad[2]>0) if(pelota[1] <= 19) pelota[1]+=velocidad[2];
            if(velocidad[2]<0) if(pelota[1] >= 0) pelota[1]+=velocidad[2];
           
            if((pelota[0]<0)||(pelota[0]>14)||(pelota[1]<0)||(pelota[1]>19)){
                teta += M_PI/2;
                if(pelota[0]<=0) {
                    pelota[0] = 0; 
                    velocidad[1] *= -1;
                }
                if(pelota[0]>=14){
                    pelota[0] = 14; 
                    velocidad[1] *= -1;
                } 
                if(pelota[1]<=0){
                    pelota[1] = 0; 
                    velocidad[2] *= -1;
                } 
                if(pelota[1]>=19){
                    pelota[1] = 19; 
                    velocidad[2]*=-1;
                } 
            }

        }
        arr[int(pelota[0])][int(pelota[1])] = " O ";
        buff = buffear(arr);
        std::cout<<"\033[2H";
        std::cout<<buff;
        std::cout<<velocidad[1]<<"|"<<velocidad[2]<<"\n";
        std::cout<<pelota[0]<<"|"<<pelota[1]<<"\n";
        std::cout<<teta;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}