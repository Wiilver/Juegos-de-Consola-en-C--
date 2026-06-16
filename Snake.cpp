#include <iostream>
#include <chrono>
#include <ctime>
#include <array>
#include <thread>
#include <random>

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

int mainLoop(){
    int tiempo, contador;
    tiempo = 1000;
    std::chrono::steady_clock::time_point inicio, actual;
    std::chrono::duration<double, std::milli> lapso;
    
    inicio = std::chrono::steady_clock::now();

    while(true){
        actual = std::chrono::steady_clock::now();
        lapso = actual-inicio;
        if(lapso.count()>tiempo){
            inicio += std::chrono::milliseconds(tiempo);
        } 
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(){
    Buffer buffer;
    Jugador jugador;
    Vector dimensiones;
    std::array<std::array<char, 10>,10> tablero;
    std::array<std::array<char, 10>,10> manzanas;

    dimensiones.x = 10;
    dimensiones.y = 10;   
    
    iniciarArreglo(tablero, dimensiones);
    iniciarArreglo(manzanas, dimensiones);
    iniciarArreglo(jugador.direcciones, dimensiones);

    jugador.coor.x = 6;
    jugador.coor.y = 4;

    jugador.direcciones[4][5] = 'd';
    jugador.direcciones[4][4] = 'd';

    iniciarBuffer(buffer.cadena, dimensiones);

    actualizarCursor(jugador.coor, dimensiones, buffer.lCeldas, buffer.cursor);

    actualizarCadena(buffer.cursor, "\033[92mO", buffer.cadena);
    actualizarCadena(buffer.cursor-10, "\033[34mO", buffer.cadena);
    actualizarCadena(buffer.cursor-20, "\033[35mO", buffer.cadena);

    std::cout<<buffer.cadena;

    return 0;
}