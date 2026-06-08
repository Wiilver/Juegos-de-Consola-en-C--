#include <iostream>
#include <array>

void iniciarArreglo(std::array<std::array<char, 3>, 3> &arreglo){
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++) arreglo[i][j] = ' ';
    }
}

void impresion(std::array<std::array<char, 3>, 3> &arreglo){
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 9; j++) std::cout<<"-";
        std::cout<<'\n';
        for(int j = 0; j < 3; j++) std::cout<<'|'<<arreglo[i][j]<<'|';
        std::cout<<'\n';
    }
    for(int j = 0; j < 9; j++) std::cout<<"-";
    std::cout<<'\n';
}

bool validarNumero(std::string respuesta){
    if((isdigit(respuesta[0])) && (respuesta.length() == 1)){
        return ((std::stoi(respuesta) < 10) && (std::stoi(respuesta) > 0));
    }
    return false;
}

bool checarPares(std::array<int, 6> posiciones, std::array<std::array<char, 3>, 3> &tablero){
    return(
        (tablero[posiciones[0]][posiciones[1]] == tablero[posiciones[2]][posiciones[3]])&&
        (tablero[posiciones[2]][posiciones[3]] == tablero[posiciones[4]][posiciones[5]])&&
        (tablero[posiciones[0]][posiciones[1]] !=' ')
    );
}

bool checarGanador(int fila, int columna, std::array<std::array<char, 3>, 3> &tablero){
    if(fila==0)      if(checarPares({0,0,0,1,0,2}, tablero)) return true;
    else if(fila==1) if(checarPares({1,0,1,1,1,2}, tablero)) return true;
    else if(fila==2) if(checarPares({2,0,2,1,2,2}, tablero)) return true;

    if(columna==0)      if(checarPares({0,0,1,0,2,0}, tablero)) return true;
    else if(columna==1) if(checarPares({0,1,1,1,2,1}, tablero)) return true;
    else if(columna==2) if(checarPares({0,2,1,2,2,2}, tablero)) return true;
    
    if(checarPares({0,0,1,1,2,2}, tablero)) return true;
    return (checarPares({2,0,1,1,0,2}, tablero));
}

int mainLoop(std::array<std::array<char, 3>, 3> &tablero){
    std::string respuesta;
    bool turnoEquis;
    int  celda, turnos, fila, columna;

    turnos = 1;

    while(true){
        if(turnos == 10) return 3;

        impresion(tablero);
        
        std::cout<<"Es el turno de las " << (turnoEquis ? 'X' : 'O') << '\n';
        std::cout<<"En que casilla desea poner una " <<(turnoEquis ? 'X' : 'O') << "? : ";
        std::cin>>respuesta;
        
        if(!validarNumero(respuesta)){
            std::cout<<"Parece que hubo un error con tu respuesta, por favor, intentelo nuevamente\n";
            continue;
        }        
        celda = std::stoi(respuesta);
        
        columna = celda%3 != 0 ? celda%3-1 : 2;
        fila = (celda%3)==0 ? celda/3-1 : int(celda/3);

        if(tablero[fila][columna]!=' '){
            std::cout<<"Ya hay un simbolo ocupando esta celda, por favor, intentelo nuevamente\n";
            continue;
        }

        tablero[fila][columna] = turnoEquis ? 'X' : 'O';
        std::cout<<"\nAcabas de poner "<<tablero[fila][columna]<<'\n';

        if(checarGanador(fila, columna, tablero)) return turnoEquis ? 1 : 2;
        turnos++;
        turnoEquis = !turnoEquis;
    }
}

int main(){
    std::array<std::array<char, 3>, 3> tablero;
    int resultado;

    iniciarArreglo(tablero);
    
    resultado = mainLoop(tablero);
    impresion(tablero);
    
    if((resultado == 1) || (resultado == 2)) std::cout<<"\nParece que en esta partida el ganador fueron las "<< (resultado==1 ? 'X' : 'O')<<"'s, bien jugado";
    else std::cout<<"\nEsta partida estuvo dificil, pero no salio ningun vencedor";
    return 0;
}