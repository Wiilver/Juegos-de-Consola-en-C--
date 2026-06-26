#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <thread>
#include <cmath>
#include <string>

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

/*
    Podriamos agregar:
    - Un selector de dificultad
    - La opcion de salir
    - La parte de reiniciar
    - Que el juego termine al llegar a 10
*/

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
    std::string personaje  = "█";
    std::string pelota     = "■";
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

struct Personaje{
    Coor posicion;
    bool movimiento = false;
    int puntuacion = 0;
};

struct Juego{
    Coor pelota;
    Personaje jugador;
    Personaje maquina;
    Vel velocidad;

    double teta = 3*M_PI/4;

    void colision(Coor &pj, Coor &pelo){
        if(pelo.y==pj.y-2)teta = M_PI/3;
        else if(pelo.y==pj.y-1)teta = M_PI/4;
        else if(pelo.y==pj.y)teta = M_PI/6;
        else if(pelo.y==pj.y+1)teta = 7*M_PI/4;
        else teta = 5*M_PI/3;
    }

    void checarColision(){
        Coor temp = pelota;
        
        temp.x = std::floor(temp.x);
        temp.y = std::floor(temp.y);

        if(temp.x==jugador.posicion.x){
            if((temp.y>=jugador.posicion.y-2)&&(temp.y<=jugador.posicion.y+2)){
                colision(jugador.posicion, temp);
                cambiarVelocidades();
                if(jugador.movimiento) velocidad.vY*=-1;
            } 
        }
        temp.x = std::ceil(pelota.x);
        temp.y = std::ceil(pelota.y);

        if(temp.x==maquina.posicion.x){
            if((temp.y>=maquina.posicion.y-2)&&(temp.y<=maquina.posicion.y+2)){
                colision(maquina.posicion, temp);
                cambiarVelocidades();
                velocidad.vX*=-1;
            }
        }
    }

    void vaciarPersonajes(Buffer &buffer){
        for(int i = 0; i < 5; i++){
            buffer.celdas[jugador.posicion.y-2+i][jugador.posicion.x] = &buffer.caracteres.vacio;
            buffer.celdas[maquina.posicion.y-2+i][maquina.posicion.x] = &buffer.caracteres.vacio;
        }
    }

    void choquePared(Buffer &buffer){
        if(pelota.y<=0){
            pelota.y = 0; 
            velocidad.vY *= -1;
        }
        if(pelota.y>=buffer.dimensiones.y-1){
            pelota.y= buffer.dimensiones.y-1; 
            velocidad.vY *= -1;
        } 
        if(pelota.x<=0){
            vaciarPersonajes(buffer);
            iniciarJuego(buffer);
            teta = ((jugador.puntuacion+maquina.puntuacion)%2==0 ? M_PI/4 : 7*M_PI/4);
            cambiarVelocidades();
            maquina.puntuacion+=1;
        } 
        if(pelota.x>=buffer.dimensiones.x-1){
            vaciarPersonajes(buffer);
            iniciarJuego(buffer);
            teta = ((jugador.puntuacion+maquina.puntuacion)%2==0 ? 3*M_PI/4 : 5*M_PI/4);
            cambiarVelocidades();
            jugador.puntuacion+=1;
        }
    }

    void cambiarPosPelota(Buffer &buffer){
        if(velocidad.vY>0) if(pelota.y >= 0) pelota.y-=velocidad.vY;
        if(velocidad.vY<0) if(pelota.y <= buffer.dimensiones.y-1) pelota.y-=velocidad.vY;
        if(velocidad.vX>0) if(pelota.x <= buffer.dimensiones.x-1) pelota.x+=velocidad.vX;
        if(velocidad.vX<0) if(pelota.x >= 0) pelota.x+=velocidad.vX;
    
        if((pelota.y<0)||(pelota.y>buffer.dimensiones.y-1)||(pelota.x<0)||(pelota.x>buffer.dimensiones.x-1)) choquePared(buffer);
    }

    void conseguirTecla(char &car){
        switch(car){
            case 'W':
            case 'w':
                car = 'W';
                break;
            case 'S':
            case 's':
                car = 'S';
                break;
            default:
                car = ' ';
                break;
        }
    }

    void checarTecla(char &tecla){
        if(_kbhit()){
            tecla = _getch();
            conseguirTecla(tecla);
        }
    }

    void cambiarPosicion(char &dir, Buffer &buffer, Personaje &pj){
        if((dir=='W')&&(pj.posicion.y > 2)){
            buffer.celdas[pj.posicion.y+2][pj.posicion.x] = &buffer.caracteres.vacio;
            pj.posicion.y--;
            buffer.celdas[pj.posicion.y-2][pj.posicion.x] = &buffer.caracteres.personaje;
        }
        else if((dir=='S')&&(pj.posicion.y < buffer.dimensiones.y-3)){
            buffer.celdas[pj.posicion.y-2][pj.posicion.x] = &buffer.caracteres.vacio;
            pj.posicion.y++;
            buffer.celdas[pj.posicion.y+2][pj.posicion.x] = &buffer.caracteres.personaje;
        }
        dir = ' ';
        pj.movimiento = true;
    }

    void imprimirPuntuaciones(int jug, int npc, int dimX){
        std::string cadena = "";

        if(dimX%3==1)      dimX+=2;
        else if(dimX%3==2) dimX+=1;

        if((dimX/3) == 1) dimX*=3;
        else if((dimX/3) == 2) dimX*=1.5;

        for(int i = 0; i < dimX+2; i++) cadena+='-';
        cadena+='\n';
        for(int i = 0; i < dimX+2; i++){
            if(i==(dimX/3)-1) {cadena+="JUG"; i+=2;}
            else if(i==(dimX/3)*2) {cadena+="NPC"; i+=2;}
            else cadena+=' ';
        }
        cadena+='\n';
        for(int i = 0; i < dimX+2; i++){
            if(i==(dimX/3)) cadena+=std::to_string(jug);
            else if(i==(dimX/3)*2+1) cadena+=std::to_string(npc);
            else cadena+=' ';
        }
        cadena+='\n';
        for(int i = 0; i < dimX+2; i++) cadena+='-';
        std::cout<<cadena;
    }

    void rellenarPersonajes(Buffer &buffer, std::string &str){
        std::string* puntero = &str;
        for(int i = 0; i < 5; i++){
            buffer.celdas[jugador.posicion.y-2+i][0] = puntero;
            buffer.celdas[maquina.posicion.y-2+i][maquina.posicion.x] = puntero;
        }
    }

    void cambiarPosMaquina(Buffer &buffer){
        if((maquina.posicion.y < pelota.y)&&(maquina.posicion.y<buffer.dimensiones.y-2.5)) maquina.posicion.y+=.4;
        else if((maquina.posicion.y > pelota.y)&&(maquina.posicion.y>2.5)) maquina.posicion.y-=.4;
    }

    bool loop(Buffer &buffer){
        iniciarJuego(buffer);
        
        char tecla = ' ';
        const int tiempo = 60;

        std::chrono::steady_clock::time_point inicio, actual;
        std::chrono::duration<double, std::milli> lapso;
        inicio = std::chrono::steady_clock::now();

        while(true){
            checarTecla(tecla);
            if(tecla!=' ') cambiarPosicion(tecla, buffer, jugador);

            actual = std::chrono::steady_clock::now();
            lapso = actual-inicio;
            if(lapso.count()>tiempo){
                inicio += std::chrono::milliseconds(tiempo);
                
                rellenarPersonajes(buffer, buffer.caracteres.vacio);
                cambiarPosMaquina(buffer);
                
                buffer.celdas[int(pelota.y)][int(pelota.x)] = &buffer.caracteres.vacio;
                cambiarPosPelota(buffer);
                checarColision();
                buffer.celdas[int(pelota.y)][int(pelota.x)] = &buffer.caracteres.pelota;
                
                rellenarPersonajes(buffer, buffer.caracteres.personaje);
                
                buffer.impresion();
                imprimirPuntuaciones(jugador.puntuacion, maquina.puntuacion, buffer.dimensiones.x);
                jugador.movimiento = false;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }


    void cambiarVelocidades(){
        velocidad.vY = sin(teta);
        velocidad.vX = cos(teta);
    }
    void iniciarJuego(Buffer &buffer){
        std::cout << "\033[2J\033[3J\033[H" << std::flush;

        pelota.y = int(buffer.dimensiones.y/2);
        pelota.x = int(buffer.dimensiones.x/2);

        jugador.posicion.y = pelota.y;
        jugador.posicion.x = 0;

        maquina.posicion.y = pelota.y;
        maquina.posicion.x = buffer.dimensiones.x-1;

        for(int i = 0; i < 5; i++){
            buffer.celdas[jugador.posicion.y-2+i][0] = &buffer.caracteres.personaje;
            buffer.celdas[maquina.posicion.y-2+i][maquina.posicion.x] = &buffer.caracteres.personaje;
        }

        buffer.celdas[pelota.y][pelota.x] = &buffer.caracteres.pelota;

        velocidad.vTotal = 1;
        cambiarVelocidades();
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
        juego.loop(buffer);

    }

    return 0;
}