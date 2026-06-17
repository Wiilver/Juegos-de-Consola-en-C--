#include <iostream>
#include <chrono>
#include <ctime>
#include <array>
#include <thread>
#include <random>
#include <vector>

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
        struct termios nuevaTermianl;

        tcgetattr(STDIN_FILENO, &terminalOriginal);

        nuevaTerminal = terminalOriginal;

        modoJuego.c_lflag &= ~(ICANON | ECHO);

        tcsetattr(STDIN_FILENO, TCSANOW, &modoJuego);
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

    //La vez pasada esta funcion era un pedote
    int _getch() {
        return getchar();
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
};

struct Jugador{
    Vector coor;
    int puntuacion;
    char direccion;
    std::array<std::array<char, 10>,10> direcciones;
};

void iniciarArreglo(std::array<std::array<char, 10>,10> &arr, Vector &dim){
    for(int i = 0; i < dim.y; i++) for(int j = 0; j < dim.x; j++) arr[i][j] = ' ';
}

void iniciarBuffer(std::string &buff, Vector &dim){
    for(int i = 0; i < dim.y; i++){
        for(int j = 0; j < dim.x; j++) buff+="\033[97m-\033[0m";
        buff+='\n';
    } 
}

void actualizarCursor(const Vector &coor,const Vector &dim, const int &grueso, int &cursor){
    cursor = coor.y*dim.x*grueso+coor.x*grueso+coor.y;
}

void actualizarCadena(int cursor, std::string nuevo, std::string &cadena){
    for(int i = 0; i < nuevo.length(); i++) cadena[cursor+i] = nuevo[i];
}

void generarManzana(std::array<std::array<char, 10>,10> &juego, Vector &dim, Vector &manzana){
    std::vector<std::array<int, 2>> manzanas;

    
    for(int i = 0; i < dim.y; i++){
        for(int j = 0; j < dim.x; j++) if(juego[i][j]==' ') manzanas.push_back({i,j});
    } 
    std::random_device semilla;
    std::mt19937 motor(semilla());
    std::uniform_int_distribution<> rango(0,manzanas.size()-1);
    
    int rnd = rango(motor);
    Vector nueva;
    nueva.y = manzanas[rnd][0];
    nueva.x = manzanas[rnd][1];
    
    manzana = nueva;
}

void conseguirTecla(char &dir){
    if(_kbhit()){
        char car = _getch();
        switch(car){
            case 'a':
            case 'A':
            case 's':
            case 'S':
            case 'd':
            case 'D':
            case 'w':
            case 'W':
            case 27:
                dir = car;
                break;
        }
    }
}

void cambiarCola(std::array<std::array<char, 10>,10> &juego, Vector &cola){
    switch(juego[cola.y][cola.x]){
        case 'W':
            juego[cola.y][cola.x] = '-';
            cola.y--;
            break;
        case 'A':
            juego[cola.y][cola.x] = '-';
            cola.x--;
            break;
        case 'S':
            juego[cola.y][cola.x] = '-';
            cola.y++;
            break;
        case 'D':
            juego[cola.y][cola.x] = '-';
            cola.x++;
            break;
    }

}

void movimiento(Buffer &buffer, Jugador &jugador, Vector &cola, Vector &dimensiones, char dir){
    actualizarCursor(jugador.coor, dimensiones, buffer.lCeldas, buffer.cursor);
    actualizarCadena(buffer.cursor, "\033[34mO", buffer.cadena);

    jugador.direcciones[jugador.coor.y][jugador.coor.x] = toupper(jugador.direccion);
    
    switch(dir){
        case 'W':
        jugador.coor.y--;
        break;
        case 'A':
        jugador.coor.x--;
        break;
        case 'S':
        jugador.coor.y++;
        break;
        case 'D':
        jugador.coor.x++;
        break;
    }
    
    actualizarCursor(jugador.coor, dimensiones, buffer.lCeldas, buffer.cursor);
    actualizarCadena(buffer.cursor, "\033[92mO", buffer.cadena);

    actualizarCursor(cola, dimensiones, buffer.lCeldas, buffer.cursor);
    actualizarCadena(buffer.cursor, "\033[97m-", buffer.cadena);

    cambiarCola(jugador.direcciones, cola);

    actualizarCursor(cola, dimensiones, buffer.lCeldas, buffer.cursor);
    actualizarCadena(buffer.cursor, "\033[35mO", buffer.cadena);
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

    iniciarBuffer(buffer.cadena, dimensiones);

    actualizarCursor(jugador.coor, dimensiones, buffer.lCeldas, buffer.cursor);

    actualizarCadena(buffer.cursor, "\033[92mO", buffer.cadena);
    actualizarCadena(buffer.cursor-buffer.lCeldas, "\033[34mO", buffer.cadena);
    actualizarCadena(buffer.cursor-buffer.lCeldas*2, "\033[35mO", buffer.cadena);

    generarManzana(jugador.direcciones, dimensiones, manzana);
    jugador.direcciones[manzana.y][manzana.x] = 'X';

    actualizarCursor(manzana, dimensiones, buffer.lCeldas, buffer.cursor);

    actualizarCadena(buffer.cursor, "\033[91mX", buffer.cadena);

    std::cout<<buffer.cadena;

    const int tiempo = 1000;
    char car;

    std::chrono::steady_clock::time_point inicio, actual;
    std::chrono::duration<double, std::milli> lapso;
    inicio = std::chrono::steady_clock::now();

    while(true){
        conseguirTecla(jugador.direccion);
        if(jugador.direccion == 27) return 0;

        actual = std::chrono::steady_clock::now();
        lapso = actual-inicio;
        if(lapso.count()>tiempo){
            inicio += std::chrono::milliseconds(tiempo);

            switch(jugador.direccion){
                case 'W':
                case 'w':
                    if(jugador.coor.y < 1) return true;
                    movimiento(buffer, jugador, cola, dimensiones, 'W');
                    break;
                case 'A':
                case 'a':
                    if(jugador.coor.x < 1) return true;
                    movimiento(buffer, jugador, cola, dimensiones, 'A');
                    break;
                case 'S':
                case 's':
                    if(jugador.coor.y > dimensiones.y-2) return true;
                    movimiento(buffer, jugador, cola, dimensiones, 'S');
                    break;
                case 'D':
                case 'd':
                    if(jugador.coor.x > dimensiones.x-2) return true;
                    movimiento(buffer, jugador, cola, dimensiones, 'D');
                    break;
            }
            std::cout<<"\033[H";
            std::cout<<buffer.cadena;
            std::cout<<'\n'<<cola.x<<" | "<<cola.y<<'\n';
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(){
    std::cout << "\033[2J\033[3J\033[H" << std::flush;
    mainLoop();
    return 0;
}