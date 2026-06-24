#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <thread>
#include <cmath>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    void configurarConsola(){
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
    void iniciarTerminal(){}
    void reiniciarTerminal(){}
#else
    #include <clocale>
    #include <unistd.h>
    #include <termios.h>
    #include <sys/select.h>

    struct termios terminalOriginal;

    void configurarConsola(){
        std::setlocale(LC_ALL, "");
    }

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

    int _getch() {
        char ch;
        read(0, &ch, 1);
        return ch;
    }
#endif

struct Coor{
    double y;
    double x;
};

struct Vel{
    double vTotal;
    double vY;
    double vX;
};

struct Caracteres{
    std::string vertical   = "│";
    std::string horizontal = "─";
    std::string esquinaNE  = "┐";
    std::string esquinaNO  = "┌";
    std::string esquinaSO  = "└";
    std::string esquinaSE  = "┘";
    std::string cruceC     = "┼";
    std::string cruceS     = "┴";
    std::string cruceE     = "┤";
    std::string cruceN     = "┬";
    std::string cruceO     = "├";
    std::string vacio      = " ";
    std::string personaje  = "#";
    std::string pelota     = "O";
};

class Buffer{
    private:
        int alto;
        int ancho;
        std::string cadena = "";
        
        void iniciarDimensiones(){
            if((alto<=0)||(ancho<=0)) throw "No se puede crear un buffer con estas dimensiones";
            dimensiones.y = alto;
            dimensiones.x = ancho;
        }
        
        void iniciarVector(){
            for(int i = 0; i < dimensiones.y+2; i++){
                std::vector<std::string*> temp;
                for(int j = 0; j < dimensiones.x+2; j++) temp.push_back(&caracteres.vacio);
                celdas.push_back(temp);
            }
        }

        void iniciarBuffer(){
            iniciarDimensiones();
            iniciarVector();
        }

        void actualizarCadena(){
            cadena = "";
            for(int i = 0; i < dimensiones.y+2; i++){
                for(int j = 0; j < dimensiones.x+2; j++){
                    if((i==0)&&(j==0)) cadena+=caracteres.esquinaNO;
                    else if((i==dimensiones.y+1)&&(j==0)) cadena+=caracteres.esquinaSO;
                    else if((i==0)&&(j==dimensiones.x+1)) cadena+=caracteres.esquinaNE;
                    else if((i==dimensiones.y+1)&&(j==dimensiones.x+1)) cadena+=caracteres.esquinaSE;
                    else if((i==0)||(i==dimensiones.y+1)) cadena+=caracteres.horizontal;
                    else if((j==0)||(j==dimensiones.x+1)) cadena+=caracteres.vertical;
                    else cadena+=*celdas[i-1][j-1];
                }
                cadena+='\n';
            }
        }

    public:
        Coor dimensiones;
        Caracteres caracteres;
        std::vector<std::vector<std::string*>> celdas;
        
        Buffer(int cAlto, int cAncho){
            alto = cAlto;
            ancho = cAncho;
            iniciarBuffer();
        }

        void impresion(){
            actualizarCadena();
            std::cout<<"\033[H";
            std::cout<<cadena;
        }
};

struct Juego{
    Coor pelota;
    Coor jugador;
    Coor maquina;
    Vel velocidad;

    int punJugador = 0;
    int punMaquina = 0;

    double teta = 3*M_PI/4;

    void choquePared(Coor &dim){
        if(pelota.y<=0){
            pelota.y = 0; 
            velocidad.vY *= -1;
        }
        if(pelota.y>=dim.y-1){
            pelota.y= dim.y-1; 
            velocidad.vY *= -1;
        } 
        if(pelota.x<=0){
            pelota.x = 0; 
            velocidad.vX *= -1;
        } 
        if(pelota.x>=dim.x-1){
            pelota.x = dim.x-1; 
            velocidad.vX*=-1;
        }
    }

    void cambiarPosPelota(Coor &dim){
        if(velocidad.vY>0) if(pelota.y >= 0) pelota.y-=velocidad.vY;
        if(velocidad.vY<0) if(pelota.y <= dim.y-1) pelota.y-=velocidad.vY;
        if(velocidad.vX>0) if(pelota.x <= dim.x-1) pelota.x+=velocidad.vX;
        if(velocidad.vX<0) if(pelota.x >= 0) pelota.x+=velocidad.vX;
    
        if((pelota.y<0)||(pelota.y>dim.y-1)||(pelota.x<0)||(pelota.x>dim.x-1)) choquePared(dim);
    }

    bool loop(Buffer &buffer){
        const int tiempo = 100;

        std::chrono::steady_clock::time_point inicio, actual;
        std::chrono::duration<double, std::milli> lapso;
        inicio = std::chrono::steady_clock::now();

        while(true){
            buffer.celdas[int(pelota.y)][int(pelota.x)] = &buffer.caracteres.vacio;
            actual = std::chrono::steady_clock::now();
            lapso = actual-inicio;
            if(lapso.count()>tiempo){
                inicio += std::chrono::milliseconds(tiempo);
                cambiarPosPelota(buffer.dimensiones);
            }
            buffer.celdas[int(pelota.y)][int(pelota.x)] = &buffer.caracteres.pelota;
            buffer.impresion();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void iniciarJuego(Buffer &buffer){
        std::cout << "\033[2J\033[3J\033[H" << std::flush;

        pelota.y = int(buffer.dimensiones.y/2);
        pelota.x = int(buffer.dimensiones.x/2);

        jugador.y = pelota.y;
        jugador.x = 0;

        maquina.y = pelota.y;
        maquina.x = buffer.dimensiones.x-1;

        for(int i = 0; i < 5; i++){
            buffer.celdas[jugador.y-2+i][0] = &buffer.caracteres.personaje;
            buffer.celdas[maquina.y-2+i][maquina.x] = &buffer.caracteres.personaje;
        }

        buffer.celdas[pelota.y][pelota.x] = &buffer.caracteres.pelota;

        velocidad.vTotal = 1;
        velocidad.vY = sin(teta);
        velocidad.vX = cos(teta);

        loop(buffer);
    }
};

Buffer crearBuffer(){
    int largo;
    int alto;
    while(true){
        try{
            std::cout<<"Por favor, ingrese que tan largo quiere que sea el mapa, el minimo es 5 : ";
            std::cin>>largo;
            if(largo<6) throw;

            std::cout<<"Por favor, ingrese que tan alto quiere que sea el mapa, el minio es 6 : ";
            std::cin>>alto;
            if(alto<7) throw;

            return Buffer(alto, largo);
        }
        catch(...){
            std::cout<<"\nParece que hubo un error, por favor, intentelo de nuevo\n";
        }
    }
}

int main(){
    iniciarTerminal();
    configurarConsola();
    while(true){
        Buffer buffer = crearBuffer();
        Juego juego;
        juego.iniciarJuego(buffer);

    }

    return 0;
}