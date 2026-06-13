#include <iostream>
#include <array>
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

struct coordenadas{
    std::array<int,2> ant = {0,0};
    std::array<int,2> nvo = {0,0};
};

struct buffer{
    std::string lienzo = "";
    coordenadas coordenadas;
    const int dimensiones[2] = {7,11};
};

struct juego{
    std::array<std::array<char, 3>,3> tablero;
    coordenadas coordenadas;
    const int dimensiones[2] = {3,3}; 
};


//En esta linea las coordenadas estan hardcodeadas
void actualizarCoordenadas(juego juego, buffer &buffer){
    buffer.coordenadas.ant = {juego.coordenadas.ant[0]*2+1, juego.coordenadas.ant[1]*4+1};
    buffer.coordenadas.nvo = {juego.coordenadas.nvo[0]*2+1, juego.coordenadas.nvo[1]*4+1};
}

void iniciarBuffer(buffer &buffer){
    for(int i = 0; i < buffer.dimensiones[0]; i++){
        if(i%2==0) for(int j = 0; j < buffer.dimensiones[1]; j++) buffer.lienzo+="\033[97m-\033[0m";
        else for(int j = 0; j < buffer.dimensiones[1]; j++) buffer.lienzo+= (j%2==0) ? "\033[97m|\033[0m" : "\033[97m \033[0m";
        buffer.lienzo+='\n';
    }
}

std::string devolverColoreado(char car){
    switch(car){
        case ' ':
            return "\033[34m ";
        case 'O':
            return "\033[94mO";
        case 'X':
            return "\033[91mX";
    }
    return "\033[34m ";
}

void cambiarBuffer(buffer &buffer, char nvo, char ant){
    std::string cadena;

    cadena = devolverColoreado(ant);
    int inicio = buffer.coordenadas.ant[0]*buffer.dimensiones[1]*10+buffer.coordenadas.ant[1]*10+buffer.coordenadas.ant[0]+4;
    for(int i = 0; i < 6; i++) buffer.lienzo[inicio+i] = cadena[i];

    
    cadena = devolverColoreado(nvo);
    inicio = buffer.coordenadas.nvo[0]*buffer.dimensiones[1]*10+buffer.coordenadas.nvo[1]*10+buffer.coordenadas.nvo[0]+4;
    for(int i = 0; i < 6; i++) buffer.lienzo[inicio+i] = cadena[i];
}

void manejarMovimiento(juego &juego, char car){
    switch(car){
        case 'W':
        case 'w':
            if(juego.coordenadas.nvo[0]>0) juego.coordenadas.nvo[0]--;
            break;
        case 'S':
        case 's':
            if(juego.coordenadas.nvo[0]<juego.dimensiones[0]-1) juego.coordenadas.nvo[0]++;
            break;
        case 'A':
        case 'a':
            if(juego.coordenadas.nvo[1]>0) juego.coordenadas.nvo[1]--;
            break;
        case 'D':
        case 'd':
            if(juego.coordenadas.nvo[1]<juego.dimensiones[1]-1) juego.coordenadas.nvo[1]++;
            break;
    }
}

void iniciarTablero(juego &juego){
    for(int i = 0; i < juego.dimensiones[0]; i++) for(int j = 0; j < juego.dimensiones[1]; j++) juego.tablero[i][j] = ' ';
}

bool checarGanador(juego juego){
    int y = juego.coordenadas.nvo[0];
    int x = juego.coordenadas.nvo[1];

    if((juego.tablero[y][0] == juego.tablero[y][1])&&(juego.tablero[y][1]==juego.tablero[y][2])&&(juego.tablero[y][0]!=' ')) return true;
    if((juego.tablero[0][x] == juego.tablero[1][x])&&(juego.tablero[1][x]==juego.tablero[2][x])&&(juego.tablero[0][x]!=' ')) return true;

    if(y==x) if((juego.tablero[0][0] == juego.tablero[1][1])&&(juego.tablero[1][1]==juego.tablero[2][2])&&(juego.tablero[0][0]!=' ')) return true;
    if((y+x)==2) if((juego.tablero[0][2] == juego.tablero[1][1])&&(juego.tablero[1][1]==juego.tablero[2][0])&&(juego.tablero[0][2]!=' ')) return true;
    return false;
}

int mainLoop(){
    //Creo que si quisiera implementar colores tendria que cambiar todo xd
    const std::string BLANCO = "\033[0m";
    const std::string ROJO   = "\033[31m";
    const std::string AZUL   = "\033[34m";

    int turno = 1;

    bool turnoEquis = true;

    char car, ant, nvo;

    ant = ' ';
    nvo = 'X';

    buffer buffer;
    juego juego;

    juego.coordenadas.ant = {1,1};
    juego.coordenadas.nvo = {1,1};
    
    iniciarTablero(juego);
    iniciarBuffer(buffer);
    
    actualizarCoordenadas(juego, buffer);
    cambiarBuffer(buffer, nvo, ant);
    std::cout<<buffer.lienzo;

//Pensar si usar metodos internos para evitar escribir tantas coas 
    while(true){
        if(turno==10) return 3;
        if(_kbhit()){
            car = _getch();
            if((car=='q')||(car=='Q')) break;
            nvo = turnoEquis ? 'X' : 'O';
            if(car==13){
                if(juego.tablero[juego.coordenadas.nvo[0]][juego.coordenadas.nvo[1]]==' '){
                    juego.tablero[juego.coordenadas.nvo[0]][juego.coordenadas.nvo[1]] = nvo;
                    ant = nvo;

                    juego.coordenadas.ant = juego.coordenadas.nvo;
                    actualizarCoordenadas(juego, buffer);
                    cambiarBuffer(buffer, nvo, ant);
                    
                    if(checarGanador(juego)){
                        std::cout<<"\033[H";
                        std::cout<<buffer.lienzo;
                        return (turnoEquis ? 1 : 2);
                    }

                    turnoEquis = !turnoEquis;
                    turno++;
                }
            }
            else{
                juego.coordenadas.ant = juego.coordenadas.nvo;
                manejarMovimiento(juego, car);
                
                ant = juego.tablero[juego.coordenadas.ant[0]][juego.coordenadas.ant[1]];
                
                actualizarCoordenadas(juego, buffer);
                cambiarBuffer(buffer, nvo, ant);
            }
            std::cout<<"\033[H";
            std::cout<<buffer.lienzo;
        }
    }
    return 0;
}

int main(){
    int resultado;
    char car;
    
    std::atexit(reiniciarTerminal);
    try{
        iniciarTerminal();
        while(true){
            system("cls");

            resultado = mainLoop();
            
            if((resultado == 1) || (resultado == 2)) 
            std::cout<<"\nParece que en esta partida el ganador fueron las "<< (resultado==1 ? 'X' : 'O')<<"'s, bien jugado";
            else std::cout<<"\nEsta partida estuvo dificil, pero no salio ningun vencedor";
            
            while(true){
                std::cout<<"\n\nDesea reiniciar el juego? (S/N) : ";
                std::cin>>car;
                car = toupper(car);
                if((car=='N')||(car=='S')) break;
                std::cout<<"\nParece que hubo un error con su respuesta, por favor, intentelo nuevamente";
            }
            if(car=='N')break;
        }
        reiniciarTerminal();
    }
    catch(std::exception error){
        std::cout<<"\nParece que hubo un error inesperado"<<error.what()<<'\n';
        reiniciarTerminal();
    }
    catch(...){
        std::cout<<"\nNo se ni como lo lograste hermano\n";
        reiniciarTerminal();
    }

    return 0;
}