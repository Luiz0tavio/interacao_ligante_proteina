/* 
 * File:   main.c
 * Author: LuizOtavio
 *
 * Created on 21 de Novembro de 2015, 01:27
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 500

typedef struct prot_st {
    // point[] refere-se à cordenada da da Proteína:
    // Sendo:
    //  - point[0] = x
    //  - point[1] = y
    //  - point[2] = z
    double point[3];
} Protein;

typedef struct cube_str {
    // max/min referem-se às cordenadas de um certo cubo:
    // Sendo:
    //  - max/min [0] = x
    //  - max/min [1] = y
    //  - max/min [2] = z
    double max[3];
    double min[3];
} Cube;

//typedef struct Leaf* Pointer;

typedef struct leaf_st {
    // boolena que verifica se a folha é de fato
    // uma folha ou um nó (se == 1 -> folha)
    int is_leaf;
    // ponteiro que aponta para essa struct
    // ou seja, dentro de Leaf, temos um 
    // "array de Leaf" com 8 filhos.
    struct leaf_st* sons[8];
    // cada folha também terá uma proteína.
    Protein protein;
    // cada folha também terá coordenadas de máx e mín
    // para saber-mos onde procurar por determinada proteína.
    Cube coords;
} Leaf;

void setCubeCoords(Leaf *leaf) {

    char str[MAX];

    // seta o valor MÍNIMO de x,y,z para o cubo
    fgets(str, sizeof (str), stdin);
    sscanf(str, " %lf %lf %lf", &leaf->coords.min[0], &leaf->coords.min[1], &leaf->coords.min[2]);

    // seta o valor MÁXIMO de x,y,z para o cubo
    fgets(str, sizeof (str), stdin);
    sscanf(str, " %lf %lf %lf", &leaf->coords.max[0], &leaf->coords.max[1], &leaf->coords.max[2]);
}

Protein getNewProtein(Leaf *leaf, char* str) {

    Protein new_prot;
    char aux[MAX];

    // armazena os pontos da proteína
    // (o aux é apenas para ignorar os nomes
    // tenho sérios problemas com regex)
    sscanf(str, " %s %s %lf %lf %lf", aux, aux, &new_prot.point[0], &new_prot.point[1], &new_prot.point[2]);
    // retorna a nova proteína para que seja
    // inserida na árvore.
    return new_prot;
}

Leaf* findLeaf(Leaf* root, double* point) {

    int c;
    Leaf *aux;

    // Função Recursiva:
    // procura na arvore o filho com
    // as coordenadas passadas no 'point'.
    // ao encontrar, se for uma folha, retorna
    // para que seja inserida, se for um nó, 
    // procura entre seus filhos.
    if(root->is_leaf){
	return root;
    }
    aux = root;
    for (c = 0; c < 8; c++) {
	if (aux->sons[c] != NULL &&
		point[0] < aux->sons[c]->coords.max[0] &&
		point[1] < aux->sons[c]->coords.max[1] &&
		point[2] < aux->sons[c]->coords.max[2] &&
		
		point[0] > aux->sons[c]->coords.min[0] &&
		point[1] > aux->sons[c]->coords.min[1] &&
		point[2] > aux->sons[c]->coords.min[2] ) {

	    if (aux->sons[c]->is_leaf) return aux->sons[c];
	    else findLeaf(aux->sons[c], point);
	}
    }
}

/*
              *-----------*
            /     /     / |
          / - - / - - /   |
        /     /     / |   |
      *-----------*   | / |
      |     |     |   |   /
      |     |     | / |   *
      | - - - - - |   | /
      |     |     |   /
      |     |     | /
      *-----------*
}*/


void splitCubes(Leaf* leaf, double* half_edge) {

    int c;
    Cube lower, upper;
   
    lower.min[0] = leaf->coords.min[0];
    lower.min[1] = leaf->coords.min[1];
    lower.min[2] = leaf->coords.min[2];
    lower.max[0] = leaf->coords.max[0] - half_edge[0];
    lower.max[1] = leaf->coords.max[1] - half_edge[1];
    lower.max[2] = leaf->coords.max[2] - half_edge[2];
    
    upper.max[0] = leaf->coords.max[0];
    upper.max[1] = leaf->coords.max[1];
    upper.max[2] = leaf->coords.max[2]; 
    upper.min[0] = leaf->coords.min[0] + half_edge[0];
    upper.min[1] = leaf->coords.min[1] + half_edge[1];
    upper.min[2] = leaf->coords.min[2] + half_edge[2];
    
    // Aloca memória para todos os filhos da folha
    // e transforma ela em um NÓ.
    for(c = 0; c < 4; c++){
        leaf->sons[c] = (Leaf *) malloc(sizeof (Leaf));
        leaf->sons[c]->coords = lower;  
        leaf->sons[c]->is_leaf = 1;
        leaf->sons[c]->protein.point[0] = MAX;
    }
    
    for(c = 4; c < 8; c++){
        leaf->sons[c] = (Leaf *) malloc(sizeof (Leaf));
        leaf->sons[c]->coords = upper;  
        leaf->sons[c]->is_leaf = 1;
        leaf->sons[c]->protein.point[0] = MAX;
    }
    
    
    for(c = 0; c < 4; c++){
        leaf->sons[c+1]->coords.max[c] += half_edge[c];
        leaf->sons[c+1]->coords.min[c] += half_edge[c];
    }
    
    for(c = 0; c < 4; c++){
        leaf->sons[c+4]->coords.max[c] -= half_edge[c];
        leaf->sons[c+4]->coords.min[c] -= half_edge[c];
    }
}


void setLeafProtein(Leaf* leaf, Protein new_protein) {

    double half_edge[3];
    Protein protein, pro_aux;
    Leaf *new_leaf;

    if (leaf->protein.point[0] == MAX) {
	leaf->protein = new_protein;
    } else {

	half_edge[0] = (leaf->coords.max[0] - leaf->coords.min[0]) / 2;
	half_edge[1] = (leaf->coords.max[1] - leaf->coords.min[1]) / 2;
	half_edge[2] = (leaf->coords.max[2] - leaf->coords.min[2]) / 2;

	protein = leaf->protein;
        leaf->is_leaf = 0;
	splitCubes(leaf, half_edge);
        
	new_leaf = findLeaf(leaf, protein.point);
	setLeafProtein(new_leaf, protein);
        
        new_leaf = findLeaf(leaf, new_protein.point);
	setLeafProtein(new_leaf, new_protein);
        
    }
}

int main(int argc, char** argv) {

    double cubeLig_edge;
    char str[MAX], lig_name[MAX], aux[11], type[30];
    Leaf *root, *leaf;
    int a;
    Protein new_protein;

    root = (Leaf *) malloc(sizeof (Leaf));
    root->is_leaf = 1;
    root->protein.point[0] = MAX;

    // armazena o valor da aresta do cubo em volta de cada ligante
    fgets(str, sizeof (str), stdin);
    sscanf(str, " %lf", &cubeLig_edge);

    // enquanto a linha 'Nome: nome_lig' 
    // não receber -1 faz:
    while (aux[0] != '-' && aux[1] != '1') {

	// recebe o nome do ligante e o armazena em
	// 'lig_name'.
	fgets(str, sizeof (str), stdin);
	sscanf(str, " %s %s", aux, lig_name);

	// lê e insere no cubo alocado as coordenadas
	// de seus pontos extremos na raiz.
	setCubeCoords(root);

	fgets(str, sizeof (str), stdin);
	while (str[0] != 'N') {

	    // cria uma nova proteína:
	    //  - aloca espaço para ela
	    //  - recebe as coordenadas do stdin
	    new_protein = getNewProtein(root, str);
	    // percorre a raiz e encontra a folha
	    // referente às coordenadas da nova proteína.
	    leaf = findLeaf(root, new_protein.point);

	    setLeafProtein(leaf, new_protein);
	    fgets(str, sizeof (str), stdin);
	}
    }
    return (EXIT_SUCCESS);
}