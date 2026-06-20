#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <exception>
#include <cstdlib>
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
    int x;
    int y;
};

struct Caracteres{
    std::string vertical   = " │ ";
    std::string horizontal = "───";
    std::string esquinaNE  = "─┐ ";
    std::string esquinaNO  = " ┌─";
    std::string esquinaSO  = " └─";
    std::string esquinaSE  = "─┘ ";
    std::string cruceC     = "─┼─";
    std::string cruceS     = "─┴─";
    std::string cruceE     = "─┤ ";
    std::string cruceN     = "─┬─";
    std::string cruceO     = " ├─";
    std::string vacio      = "   ";
    std::string salto      = "\n";
    std::string personaje  = " O ";
    std::string flag   = "\033[91m■\033[0m";
};

struct Numeros{
    std::string uno    = "\033[1;94m1\033[0m";
    std::string dos    = "\033[32m2\033[0m";
    std::string tres   = "\033[1;91m3\033[0m";
    std::string cuatro = "\033[34m4\033[0m";
    std::string cinco  = "\033[31m5\033[0m";
    std::string seis   = "\033[36m6\033[0m";
    std::string siete  = "\033[35m7\033[0m";
    std::string ocho   = "\033[38;2;133, 133, 133;Bm8\033[0m";
    std::string nueve  = "\033[97m9\033[0m";
    
};

class Buffer{
    private:
        std::string cadena = "";
        
        void iniciarDimensiones(){
            dimensiones.y = 8;
            dimensiones.x = 8;
        }
        
        void iniciarCursor(){
            cursor.x = dimensiones.x%2 == 0 ? dimensiones.x-1 : dimensiones.x;
            cursor.y = dimensiones.y%2 == 0 ? dimensiones.y-1 : dimensiones.y;
        }


        void iniciarVector(){
            for(int i = 0; i < dimensiones.y*2+1; i++){
                std::vector<std::string*> arr;
                for(int j = 0; j < dimensiones.x*2+2; j++){
                    std::string* temp = nullptr;
                    
                    if(j==dimensiones.x*2+1)                temp = &caracter.salto;
                    else if((j%2==1)&&(i%2==0))             temp = &caracter.horizontal;
                    else if((i%2==1)&&(j%2==0))             temp = &caracter.vertical;
                    else if((i%2==1)&&(j%2==1))             temp = &caracter.vacio;
                    else if((j==dimensiones.x*2)&&(i%2==0)) temp = &caracter.cruceE;
                    else if((j==0)&&(i%2==0))               temp = &caracter.cruceO;
                    else if((i==0)&&(j%2==0))               temp = &caracter.cruceN;
                    else if((i==dimensiones.y*2)&&(j%2==0)) temp = &caracter.cruceS;
                    else                                    temp = &caracter.cruceC;

                    arr.push_back(temp);
                }
                celdas.push_back(arr);
            }
            celdas[0][0] = &caracter.esquinaNO;
            celdas[celdas.size()-1][0] = &caracter.esquinaSO;
            celdas[0][celdas[0].size()-2] = &caracter.esquinaNE;
            celdas[celdas.size()-1][celdas[0].size()-2] = &caracter.esquinaSE;
            celdas[cursor.y][cursor.x] = &caracter.personaje;
        }

    public:
        Caracteres caracter;
        Numeros color;

        Coor dimensiones;
        Coor cursor;

        std::vector<std::vector<std::string*>> celdas;

        void iniciarBuffer(){
            iniciarDimensiones();
            iniciarCursor();
            iniciarVector();
        }
        
        void actualizarCadena(){
            cadena = "";
            for(int i = 0; i < celdas.size(); i++){
                for(int j = 0; j < celdas[0].size(); j++){
                    cadena+=*celdas[i][j];
                }
            }
        }

        void actualizarCursor(char dir){
            switch(dir){
                case 'W':
                    cursor.y-=2;
                    break;
                case 'A':
                    cursor.x-=2;
                    break;
                case 'S':
                    cursor.y+=2;
                    break;
                case 'D':
                    cursor.x+=2;
                    break;
            }
        }

        void impresion(){
            std::cout<<cadena;
        }

        bool checarFlag(){
            return *celdas[cursor.y][cursor.x] == caracter.vacio;
        }
};

char conseguirTecla(){
    if(_kbhit()){
        char car = _getch();

        switch(car){
            case 'W':
            case 'w':
                return 'W';
            case 'A':
            case 'a':
                return 'A';
            case 'S':
            case 's':
                return 'S';
            case 'D':
            case 'd':
                return 'D';
            case 'P':
            case 'p':
                return 'P';
            case 27:
                return 27;
        }
    }
    return ' ';
}

void crearMinas(std::vector<std::vector<char>> &minas, Coor dim, Coor coor){
    std::vector<std::array<int, 2>> rand;
    double num = dim.y*dim.x/6;
    int superior = dim.y-1;
    num = std::ceil(num);

    std::random_device semilla;
    std::mt19937 motor(semilla());
    
    for(int i = 0; i < dim.y; i++) {
        for(int j = 0; j < dim.x; j++){
            if(dim.y*dim.x > 15){
                if((((i==coor.y+1)||(i==coor.y-1))&&(((j > coor.x-2)||(j < coor.x+2))))||
                    ((i==coor.y)&&((j==coor.x-1)||(j==coor.x+1))))
                
            }

            rand.push_back({i,j});
        } 
    }
    
    while(rand.size() > num){
        std::uniform_int_distribution<> rango(0,superior);
        int rnd= rango(motor);
        rand.erase(rand.begin()+rnd);
        superior--;
    }

    for(int i = 0; i < rand.size(); i++) minas[rand[i][0]][rand[i][1]] = 'X';
}

int main(){
    char car = ' ';

    int puntuacion = 0;
    
    std::vector<std::vector<char>> minas;
    
    Buffer buffer;
    std::string ant = "";
    
    configurarConsola();
    std::cout << "\033[2J\033[3J\033[H" << std::flush;

    buffer.iniciarBuffer();
    buffer.actualizarCadena();
    buffer.impresion();


    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        car = ' ';
        car = conseguirTecla();
        
        if(car == ' ') continue;
        if(car == 'P'){
            if(ant == buffer.caracter.vacio) ant = buffer.caracter.flag;
            continue;
        }
        if(car == 27){
            if(ant != buffer.caracter.vacio) continue;
            if(!puntuacion) crearMinas(minas, buffer.dimensiones, buffer.cursor);
        }
        buffer.celdas[buffer.cursor.y][buffer.cursor.x] = &ant;
        
        buffer.actualizarCursor(car);
        
        ant = *buffer.celdas[buffer.cursor.y][buffer.cursor.x];
        buffer.celdas[buffer.cursor.y][buffer.cursor.x] = &buffer.caracter.personaje;

        buffer.actualizarCadena();
        std::cout<<"\033[H";
        buffer.impresion();
    }


    return 0;
}