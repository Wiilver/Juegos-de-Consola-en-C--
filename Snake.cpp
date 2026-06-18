#include <iostream>
#include <chrono>
#include <ctime>
#include <array>
#include <thread>
#include <random>
#include <vector>
#include <exception>
#include <cstdlib>

#ifdef _WIN32
    #include <conio.h>
    void iniciarTerminal(){}
    void reiniciarTerminal(){}
#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/select.h>

    struct termios terminalOriginal;

    void iniciarTerminal(){
        struct termios nuevaTerminal;

        tcgetattr(STDIN_FILENO, &terminalOriginal);

        nuevaTerminal = terminalOriginal;

        nuevaTerminal.c_lflag &= ~(ICANON | ECHO);
        nuevaTerminal.c_iflag &= ~(IGNCR | ICRNL | INLCR);
        
        tcsetattr(STDIN_FILENO, TCSANOW, &nuevaTerminal);
    }

    void reiniciarTerminal(){
        tcsetattr(STDIN_FILENO, TCSANOW, &terminalOriginal);
    }

    int _kbhit() {
        struct timeval tiempo = { 0L, 0L };
        fd_set registros;
        FD_ZERO(&registros);
        FD_SET(0, &registros);
        return select(1, &registros, NULL, NULL, &tiempo);
    }

    //La vez pasada esta funcion era incluso mas rara
    int _getch() {
        char ch;
        read(0, &ch, 1);
        return ch;
    }
#endif

struct Vector{
    int y;
    int x;
};

struct Buffer{
    std::string cadena;
    int cursor;
    const int lCeldas = 10;
    const std::string manzana = "\033[91mX";
    const std::string fondo = "\033[97m-";

    void actualizarCursor(const Vector &coor,const Vector &dim){
       cursor = coor.y*dim.x*lCeldas+coor.x*lCeldas+coor.y;
    }

    void actualizarCadena(std::string nuevo){
        for(int i = 0; i < nuevo.length(); i++) cadena[cursor+i] = nuevo[i];
    }

    void actualizarBuffer(const Vector &coor,const Vector &dim, std::string nuevo){
        actualizarCursor(coor, dim);
        actualizarCadena(nuevo);
    }

};

struct Jugador{
    Vector coor;
    int puntuacion;
    char direccion;
    std::array<std::array<char, 10>,10> direcciones;
    const std::string cabeza = "\033[92mO";
    const std::string cuerpo = "\033[34mO";
    const std::string cola = "\033[35mO";
};

void iniciarArreglo(std::array<std::array<char, 10>,10> &arr, Vector &dim){
    for(int i = 0; i < dim.y; i++) for(int j = 0; j < dim.x; j++) arr[i][j] = ' ';
}

void iniciarBuffer(std::string &buff, Vector &dim, std::string fondo){
    for(int i = 0; i < dim.y; i++){
        for(int j = 0; j < dim.x; j++){
            buff+=fondo;
            buff+="\033[0m";
        }
        buff+='\n';
    } 
}

bool generarManzana(std::array<std::array<char, 10>,10> &juego, Vector &dim, Vector &manzana){
    std::vector<std::array<int, 2>> manzanas;
    
    for(int i = 0; i < dim.y; i++){
        for(int j = 0; j < dim.x; j++) if(juego[i][j]==' ') manzanas.push_back({i,j});
    } 
    
    if(manzanas.size()==0) return true;

    static std::random_device semilla;
    static std::mt19937 motor(semilla());
    std::uniform_int_distribution<> rango(0,manzanas.size()-1);
    
    int rnd = rango(motor);
    Vector nueva;
    nueva.y = manzanas[rnd][0];
    nueva.x = manzanas[rnd][1];
    
    manzana = nueva;
    return false;
}

void conseguirTecla(Jugador &jugador){
    if(_kbhit()){
        char car = _getch();
        switch(car){
            case 'a':
            case 'A':
                if(jugador.direcciones[jugador.coor.y][jugador.coor.x-1]=='D') break;
                jugador.direccion = 'A';
                break;
            case 's':
            case 'S':
                if(jugador.direcciones[jugador.coor.y+1][jugador.coor.x]=='W') break;
                jugador.direccion = 'S';
                break;
            case 'd':
            case 'D':
                if(jugador.direcciones[jugador.coor.y][jugador.coor.x+1]=='A') break;
                jugador.direccion = 'D';
                break;
            case 'w':
            case 'W':
                if(jugador.direcciones[jugador.coor.y-1][jugador.coor.x]=='S') break;
                jugador.direccion = 'W';
                break;
            case 27:
                jugador.direccion = 27;
                break;
        }
    }
}

void cambiarCola(std::array<std::array<char, 10>,10> &juego, Vector &cola){
    switch(juego[cola.y][cola.x]){
        case 'W':
            juego[cola.y][cola.x] = ' ';
            cola.y--;
            break;
        case 'A':
            juego[cola.y][cola.x] = ' ';
            cola.x--;
            break;
        case 'S':
            juego[cola.y][cola.x] = ' ';
            cola.y++;
            break;
        case 'D':
            juego[cola.y][cola.x] = ' ';
            cola.x++;
            break;
    }

}

int movimiento(Buffer &buffer, Jugador &jugador, Vector &cola, Vector &dimensiones, char dir){
    //0 no paso nada
    //1 comio manzana
    //2 perdio
    int equis, ye;

    ye = jugador.coor.y;
    equis = jugador.coor.x;

    buffer.actualizarBuffer(jugador.coor, dimensiones, jugador.cuerpo);

    jugador.direcciones[ye][equis] = toupper(jugador.direccion);
    switch(dir){
        case 'W':
            ye--;
            break;
        case 'A':
            equis--;
            break;
        case 'S':
            ye++;
            break;
        case 'D':
            equis++;
            break;
    }
    if(jugador.direcciones[ye][equis]!='X'){
        if(isalpha(jugador.direcciones[ye][equis])) return 2;
    }

    jugador.coor.y = ye;
    jugador.coor.x = equis;

    buffer.actualizarBuffer(jugador.coor, dimensiones, jugador.cabeza);

    if(jugador.direcciones[ye][equis]=='X') return 1;

    buffer.actualizarBuffer(cola, dimensiones, buffer.fondo);

    cambiarCola(jugador.direcciones, cola);

    buffer.actualizarBuffer(cola, dimensiones, jugador.cola);
    return 0;
}

int moverse(Buffer &buffer, Jugador &jugador, Vector &cola, Vector &dimensiones, Vector &manzana){
    //0 no paso nada
    //1 gano
    //2 perdio
    int res = movimiento(buffer, jugador, cola, dimensiones, jugador.direccion); 
    if(res == 1){
        if(generarManzana(jugador.direcciones, dimensiones, manzana)) return 1;
        jugador.direcciones[manzana.y][manzana.x] = 'X';
        buffer.actualizarBuffer(manzana, dimensiones, buffer.manzana);
    }
    if(res == 2) return 2;
    return 0;
}

int mainLoop(){
    Buffer buffer;
    Jugador jugador;

    Vector cola;
    Vector manzana;
    Vector dimensiones;
    
    dimensiones.x = 10;
    dimensiones.y = 10;   
    
    iniciarArreglo(jugador.direcciones, dimensiones);

    cola.x = 4;
    cola.y = 4;

    jugador.coor.x = 6;
    jugador.coor.y = 4;

    jugador.direcciones[4][5] = 'D';
    jugador.direcciones[4][4] = 'D';

    iniciarBuffer(buffer.cadena, dimensiones, buffer.fondo);

    buffer.actualizarBuffer(jugador.coor, dimensiones, jugador.cabeza);

    buffer.cursor-=buffer.lCeldas;
    buffer.actualizarCadena(jugador.cuerpo);
    buffer.cursor-=buffer.lCeldas;
    buffer.actualizarCadena(jugador.cola);

    buffer.cursor+=buffer.lCeldas*2;

    generarManzana(jugador.direcciones, dimensiones, manzana);
    jugador.direcciones[manzana.y][manzana.x] = 'X';
    buffer.actualizarBuffer(manzana, dimensiones, buffer.manzana);

    std::cout<<buffer.cadena;

    int res = 0;
    const int tiempo = 100;

    std::chrono::steady_clock::time_point inicio, actual;
    std::chrono::duration<double, std::milli> lapso;
    inicio = std::chrono::steady_clock::now();

    while(true){
        conseguirTecla(jugador);
        if(jugador.direccion == 27) return 0;

        actual = std::chrono::steady_clock::now();
        lapso = actual-inicio;
        if(lapso.count()>tiempo){
            inicio += std::chrono::milliseconds(tiempo);

            switch(jugador.direccion){
                case 'W':
                case 'w':
                    if(jugador.coor.y < 1) return true;
                    res = moverse(buffer, jugador, cola, dimensiones, manzana);
                    break;
                case 'A':
                case 'a':
                    if(jugador.coor.x < 1) return true;
                    res = moverse(buffer, jugador, cola, dimensiones, manzana);
                    break;
                case 'S':
                case 's':
                    if(jugador.coor.y > dimensiones.y-2) return true;
                    res = moverse(buffer, jugador, cola, dimensiones, manzana);
                    break;
                case 'D':
                case 'd':
                    if(jugador.coor.x > dimensiones.x-2) return true;
                    res = moverse(buffer, jugador, cola, dimensiones, manzana);
                    break;
            }
            if(res==1) return false;
            if(res==2) return true;            
            std::cout<<"\033[H";
            std::cout<<buffer.cadena;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(){
    iniciarTerminal();
    std::atexit(reiniciarTerminal);
    try{
        char resp;
        while(true){
            std::cout << "\033[2J\033[3J\033[H" << std::flush;
            if(mainLoop()) std::cout<<"\nMalo qliao perdedor";
            else std::cout<<"\nEres literalmente un ganador";
            while(true){
                std::cout<<"\nDeseas intentarlo de nuevo? (S/N) : ";
                std::cin>>resp;
                resp = toupper(resp);
                if((resp=='S')||(resp=='N')) break;
                std::cout<<"\nParece que contestaste mal, intentalo de nuevo\n";
            }
            if(resp=='N'){
                reiniciarTerminal();
                break;
            } 
        }
    }
    catch(const std::exception &e){
        std::cout<<"\nParece que ocurrio un error\n"<<e.what();
        reiniciarTerminal();
    }
    catch(...){
        std::cout<<"\nOcurrio un error rarisimo, se terminara el proceso\n";
        reiniciarTerminal();
    }
    
    return 0;
}