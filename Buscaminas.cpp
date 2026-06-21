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
    std::string flag       = "\033[91m ■ \033[0m";
};

struct Numeros{
    std::string cero   = "\033[38;2;91;91;91m 0 \033[0m";
    std::string uno    = "\033[1;94m 1 \033[0m";
    std::string dos    = "\033[32m 2 \033[0m";
    std::string tres   = "\033[1;91m 3 \033[0m";
    std::string cuatro = "\033[34m 4 \033[0m";
    std::string cinco  = "\033[31m 5 \033[0m";
    std::string seis   = "\033[36m 6 \033[0m";
    std::string siete  = "\033[35m 7 \033[0m";
    std::string ocho   = "\033[38;2;133;133;133m 8 \033[0m";
    std::string nueve  = "\033[97m 9 \033[0m";
    
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
        int nMinas = 0;
        int nCeldas = 0;
        Caracteres caracter;
        Numeros color;

        Coor dimensiones;
        Coor cursor;

        std::vector<std::vector<std::string*>> celdas;

        void iniciarBuffer(){
            iniciarDimensiones();
            iniciarCursor();
            iniciarVector();
            nCeldas = dimensiones.y*dimensiones.x;
            nMinas = std::ceil(dimensiones.y*dimensiones.x/6);
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
                    if(cursor.y > 1)cursor.y-=2;
                    break;
                case 'A':
                    if(cursor.x > 1)cursor.x-=2;
                    break;
                case 'S':
                    if(cursor.y < dimensiones.y*2-1)cursor.y+=2;
                    break;
                case 'D':
                    if(cursor.x < dimensiones.x*2-1)cursor.x+=2;
                    break;
            }
        }

        void impresion(){
            actualizarCadena();
            std::cout<<"\033[H";
            std::cout<<cadena;
        }

        void checar(int &num, std::vector<std::vector<char>> &minas, int suma, bool modo){
            cursor.x -= modo ? suma : -suma;
            if(*celdas[cursor.y][cursor.x]==caracter.vacio){
                num = checarMinas(minas);
                asignarNumero(num, minas);
                num = 0;
            } 
            cursor.x += modo ? suma : -suma;
        }

        void checarCero(std::vector<std::vector<char>> &minas){
            int num = 0;
            if(cursor.y > 2){
                cursor.y-=2;
                if(cursor.x > 2) checar(num, minas, 2, true);
                if(cursor.x < dimensiones.x*2-1) checar(num, minas, 2, false);
                checar(num, minas, 0, true);
                cursor.y+=2;
            }
            if(cursor.y<dimensiones.y*2-1){
                cursor.y+=2;
                if(cursor.x > 2) checar(num, minas, 2, true);
                if(cursor.x < dimensiones.x*2-1) checar(num, minas, 2, false);
                checar(num, minas, 0, true);
                cursor.y-=2;
            }
            if(cursor.x > 2) checar(num, minas, 2, true);
            if(cursor.x < dimensiones.x*2-1) checar(num, minas, 2, false);

        }

        void asignarNumero(int num, std::vector<std::vector<char>> &minas){
            switch(num){
                case 1:
                    celdas[cursor.y][cursor.x] = &color.uno;
                    break;
                case 2:
                    celdas[cursor.y][cursor.x] = &color.dos;
                    break;
                case 3:
                    celdas[cursor.y][cursor.x] = &color.tres;
                    break;
                case 4:
                    celdas[cursor.y][cursor.x] = &color.cuatro;
                    break;
                case 5:
                    celdas[cursor.y][cursor.x] = &color.cinco;
                    break;
                case 6:
                    celdas[cursor.y][cursor.x] = &color.seis;
                    break;
                case 7:
                    celdas[cursor.y][cursor.x] = &color.siete;
                    break;
                case 8:
                    celdas[cursor.y][cursor.x] = &color.ocho;
                    break;
                case 9:
                    celdas[cursor.y][cursor.x] = &color.nueve;
                    break;
                case 0:
                    celdas[cursor.y][cursor.x] = &color.cero;
                    break;
            }
        }

        int checarMinas(std::vector<std::vector<char>> &minas){
            Coor temp;
            temp.y = (cursor.y-1)/2;            
            temp.x = (cursor.x-1)/2;      
            int contador = 0;
            if(temp.y>0){
                if(temp.x>0){
                    if(minas[temp.y-1][temp.x-1]=='X') contador++;
                }
                if(temp.x<dimensiones.x-1){
                    if(minas[temp.y-1][temp.x+1]=='X') contador++;

                }
                if(minas[temp.y-1][temp.x]=='X') contador++;
            }
            if(temp.y<dimensiones.y-1){
                if(temp.x>0){
                    if(minas[temp.y+1][temp.x-1]=='X') contador++;
                }
                if(temp.x<dimensiones.x-1){
                    if(minas[temp.y+1][temp.x+1]=='X') contador++;
                }
                if(minas[temp.y+1][temp.x]=='X') contador++;
            }
            if(temp.x>0){
                if(minas[temp.y][temp.x-1]=='X') contador++;
            }
            if(temp.x<dimensiones.x-1){
                if(minas[temp.y][temp.x+1]=='X') contador++;
            }
            return contador;
        }
        
        void click(std::vector<std::vector<char>> &minas){
            int num = checarMinas(minas);
            asignarNumero(num, minas);
            if(num == 0) checarCero(minas);
        }

        bool checarGanador(std::vector<std::vector<char>> &minas){
            int contador = 0;
            std::string str = "";
            for(int i = 1; i < dimensiones.y*2+1; i+=2){
                for(int j = 1; j < dimensiones.x*2+1; j+=2){
                    str= *celdas[i][j];
                    if((str!=caracter.vacio)&&(str!=caracter.flag)&&(str!=caracter.personaje)) contador++;
                }
            }
            return (contador == (nCeldas-nMinas));
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
            case 13:
                return 13;
        }
    }
    return ' ';
}

void crearMinas(std::vector<std::vector<char>> &minas, Coor dim, Coor coor){
    std::vector<std::array<int, 2>> rand;
    Coor temp;

    temp.x = (coor.x-1)/2;
    temp.y = (coor.y-1)/2;
    double num = dim.y*dim.x/6;
    num = std::ceil(num);

    static std::random_device semilla;
    static std::mt19937 motor(semilla());
    for(int i = 0; i < dim.y; i++) {
        for(int j = 0; j < dim.x; j++){
            if(dim.y*dim.x > 15){
                if(((i > temp.y-2)&&(i<temp.y+2))&&(((j > temp.x-2)&&(j < temp.x+2))))continue;
            }
            rand.push_back({i,j});
        } 
    }
    
    while(num > 0){
        std::uniform_int_distribution<> rango(0,rand.size()-1);
        int rnd= rango(motor);
        minas[rand[rnd][0]][rand[rnd][1]] = 'X';
        rand.erase(rand.begin()+rnd);
        num--;
    }
}

void iniciarMinas(std::vector<std::vector<char>>&minas, Coor dim){
    for(int i = 0; i < dim.y; i++){
        std::vector<char> temp;

        for(int j = 0; j < dim.x; j++) temp.push_back(' ');
        minas.push_back(temp);
    }
}

bool mainLoop(){
    char car = ' ';

    int alrededor = 0;
    bool puntuacion = false;
    
    std::vector<std::vector<char>> minas;
    
    Buffer buffer;
    std::string* ant = &buffer.caracter.vacio;
    
    configurarConsola();
    std::cout << "\033[2J\033[3J\033[H" << std::flush;
    
    buffer.iniciarBuffer();
    buffer.impresion();
    
    iniciarMinas(minas, buffer.dimensiones);

    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        car = ' ';
        car = conseguirTecla();
        
        if(car == ' ') continue;
        if(car == 'P'){
            if(*ant == buffer.caracter.vacio) ant = &buffer.caracter.flag;
            else if(*ant == buffer.caracter.flag) ant = &buffer.caracter.vacio;
            continue;
        }
        else{
            if(car == 13){
                if(!puntuacion) {
                    crearMinas(minas, buffer.dimensiones, buffer.cursor);
                    puntuacion = true;
                }
                if(*ant == buffer.color.cero){
                    buffer.checarCero(minas); 
                    buffer.impresion();
                    if(buffer.checarGanador(minas)){
                        buffer.impresion();
                        return true;
                    }

                    continue;
                } 
                if(*ant != buffer.caracter.vacio) continue;
                if(minas[(buffer.cursor.y-1)/2][(buffer.cursor.x-1)/2]=='X') return false;
                buffer.click(minas);
                ant = buffer.celdas[buffer.cursor.y][buffer.cursor.x];
                if(buffer.checarGanador(minas)){
                    buffer.impresion();
                    return true;
                }
            }
            else{
                buffer.celdas[buffer.cursor.y][buffer.cursor.x] = ant;
                
                buffer.actualizarCursor(car);
                
                ant = buffer.celdas[buffer.cursor.y][buffer.cursor.x];
                buffer.celdas[buffer.cursor.y][buffer.cursor.x] = &buffer.caracter.personaje;
            }
            buffer.impresion();
        }
    }
}

int main(){
    
    if(mainLoop())std::cout<<"\nGanaste";
    else std::cout<<"\nPerdiste";

    return 0;
}