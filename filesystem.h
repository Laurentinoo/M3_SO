#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stddef.h>

// Enum para diferenciar arquivo e diretório
typedef enum {
    FILE_TYPE,
    DIRECTORY_TYPE
} NodeType;

// Estrutura de Arquivo
typedef struct File {
    char* name;
    char* content;
    size_t size;
} File;

// Declaração adiantada
typedef struct Directory Directory;

// Estrutura genérica de nó
typedef struct TreeNode {
    char* name;
    NodeType type;
    union {
        File* file;
        Directory* directory;
    } data;
} TreeNode;

// Árvore B
typedef struct BTree BTree;
typedef struct BTreeNode BTreeNode;

// Diretório contendo árvore B de TreeNode*
struct Directory {
    BTree* tree;
    struct Directory* parent;
};

// Criação de diretório raiz
Directory* get_root_directory();

// Funções de diretório
TreeNode* create_directory(const char* name, Directory* parent);
void delete_directory(BTree* tree, const char* name);
void change_directory(Directory** current, const char* path);
void list_directory_contents(Directory* dir);

// Funções de arquivos
TreeNode* create_txt_file(const char* name, const char* content);
void delete_txt_file(BTree* tree, const char* name);

// Funções da árvore B
BTree* btree_create(int t);
void btree_insert(BTree* tree, TreeNode* node);
void btree_delete(BTree* tree, const char* name);
TreeNode* btree_search(BTree* tree, const char* name);
void btree_traverse(BTree* tree);

void export_fs_to_img(Directory* root);
void mostrar_diretorio_atual(Directory* root, Directory* current);
void listar_diretorios(Directory* dir);

#endif