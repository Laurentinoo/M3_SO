#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h" 

#define TAM 16 //quantidade de arquivos/diretorio por diretorio

struct BTree {
    BTreeNode* root;
    int t;
};

struct BTreeNode {
    int n;
    TreeNode** keys;
    BTreeNode** C;
    int leaf;
};

BTree* btree_create(int t) {
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    if (!tree) { perror("Erro ao criar árvore B"); exit(1); }
    tree->t = t;

    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (!node) { perror("Erro ao criar nó raiz da árvore B"); exit(1); }

    node->leaf = 1;
    node->n = 0;
    node->keys = (TreeNode**)malloc(sizeof(TreeNode*) * (2 * t - 1));
    node->C = (BTreeNode**)malloc(sizeof(BTreeNode*) * (2 * t));
    tree->root = node;
    return tree;
}

Directory* get_root_directory() {
    Directory* root = (Directory*)malloc(sizeof(Directory));
    if (!root) { perror("Erro ao alocar diretório raiz"); exit(1); }
    root->tree = btree_create(TAM);
    root->parent = NULL;
    return root;
}

TreeNode* create_directory(const char* name, Directory* parent) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (!node) { perror("Erro ao alocar diretório"); exit(1); }
    node->name = strdup(name);
    node->type = DIRECTORY_TYPE;

    Directory* new_dir = (Directory*)malloc(sizeof(Directory));
    if (!new_dir) { perror("Erro ao alocar subdiretório"); exit(1); }

    new_dir->tree = btree_create(TAM);
    new_dir->parent = parent;
    node->data.directory = new_dir;
    return node;
}

TreeNode* create_txt_file(const char* name, const char* content) {
    FILE* fp = fopen(name, "w");
    if (!fp) { perror("Erro ao criar arquivo físico"); return NULL; }
    fprintf(fp, "%s", content);
    fclose(fp);

    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->name = strdup(name);
    node->type = FILE_TYPE;

    File* file = (File*)malloc(sizeof(File));
    file->name = strdup(name);
    file->content = strdup(content);
    file->size = strlen(content);
    node->data.file = file;
    return node;
}

int compare_names(const char* a, const char* b) {
    return strcmp(a, b);
}

void btree_insert_non_full(BTreeNode* node, TreeNode* new_node, int t) {
    int i = node->n - 1;
    if (node->leaf) {
        while (i >= 0 && compare_names(new_node->name, node->keys[i]->name) < 0) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = new_node;
        node->n++;
    }
}

void btree_insert(BTree* tree, TreeNode* new_node) {
    BTreeNode* r = tree->root;
    if (r->n == 2 * tree->t - 1) {
        printf("Inserção não suportada: raiz cheia\n");
        return;
    }
    btree_insert_non_full(r, new_node, tree->t);
}

TreeNode* btree_search_node(BTreeNode* node, const char* name) {
    int i = 0;
    while (i < node->n && strcmp(name, node->keys[i]->name) > 0) { i++; }
    if (i < node->n && strcmp(name, node->keys[i]->name) == 0) return node->keys[i];
    if (node->leaf) return NULL;
    return btree_search_node(node->C[i], name);
}

TreeNode* btree_search(BTree* tree, const char* name) {
    return btree_search_node(tree->root, name);
}

void btree_delete(BTree* tree, const char* name) {
    BTreeNode* node = tree->root;
    if (!node->leaf) {
        printf("Remoção só suportada com raiz folha nesta versão.\n");
        return;
    }
    int i;
    for (i = 0; i < node->n; i++) {
        if (strcmp(node->keys[i]->name, name) == 0) break;
    }
    if (i == node->n) {
        printf("Item '%s' não encontrado para remoção.\n", name);
        return;
    }
    free(node->keys[i]->name);
    if (node->keys[i]->type == FILE_TYPE) {
        free(node->keys[i]->data.file->content);
        free(node->keys[i]->data.file->name);
        free(node->keys[i]->data.file);
    } else if (node->keys[i]->type == DIRECTORY_TYPE) {
        free(node->keys[i]->data.directory->tree->root->keys);
        free(node->keys[i]->data.directory->tree->root->C);
        free(node->keys[i]->data.directory->tree->root);
        free(node->keys[i]->data.directory->tree);
        free(node->keys[i]->data.directory);
    }
    free(node->keys[i]);
    for (int j = i; j < node->n - 1; j++) {
        node->keys[j] = node->keys[j + 1];
    }
    node->n--;
}

void delete_directory(BTree* tree, const char* name) {
    TreeNode* node = btree_search(tree, name);
    if (!node) { printf("Diretório '%s' não encontrado.\n", name); return; }
    if (node->type != DIRECTORY_TYPE) { printf("'%s' não é um diretório.\n", name); return; }
    BTree* subtree = node->data.directory->tree;
    if (subtree->root->n > 0) {
        printf("Diretório '%s' não está vazio.\n", name);
        return;
    }
    btree_delete(tree, name);
    printf("Diretório '%s' removido com sucesso.\n", name);
}

void change_directory(Directory** current, const char* path) {
    if (strcmp(path, "..") == 0) {
        if ((*current)->parent != NULL) *current = (*current)->parent;
        else printf("Já está no diretório raiz.\n");
        return;
    }
    TreeNode* node = btree_search((*current)->tree, path);
    if (!node || node->type != DIRECTORY_TYPE) {
        printf("Diretório '%s' não encontrado.\n", path);
        return;
    }
    *current = node->data.directory;
}

void list_directory_contents(Directory* dir) {
    if (!dir || !dir->tree) {
        printf("Diretório inválido.\n");
        return;
    }
    btree_traverse(dir->tree);
}

void delete_txt_file(BTree* tree, const char* name) {
    TreeNode* node = btree_search(tree, name);
    if (!node) { printf("Arquivo '%s' não encontrado.\n", name); return; }
    if (node->type != FILE_TYPE) { printf("'%s' não é um arquivo.\n", name); return; }
    if (remove(name) == 0) printf("Arquivo físico '%s' removido.\n", name);
    else perror("Erro ao remover o arquivo do disco");
    btree_delete(tree, name);
    printf("Arquivo '%s' removido do sistema virtual.\n", name);
}

void btree_traverse_node(BTreeNode* node) {
    for (int i = 0; i < node->n; i++) {
        if (!node->leaf) btree_traverse_node(node->C[i]);
        TreeNode* current = node->keys[i];
        if (current->type == FILE_TYPE) printf("[ARQ] %s (%lu bytes)\n", current->name, current->data.file->size);
        else printf("[DIR] %s/\n", current->name);
    }
    if (!node->leaf) btree_traverse_node(node->C[node->n]);
}

void btree_traverse(BTree* tree) {
    if (tree && tree->root) btree_traverse_node(tree->root);
}

void export_btree_node(FILE* fp, BTreeNode* node, int depth) {
    for (int i = 0; i < node->n; i++) {
        if (!node->leaf) export_btree_node(fp, node->C[i], depth + 1);
        TreeNode* item = node->keys[i];
        for (int d = 0; d < depth; d++) fprintf(fp, "  ");
        if (item->type == FILE_TYPE)
            fprintf(fp, "[ARQ] %s (%lu bytes)\n", item->name, item->data.file->size);
        else {
            fprintf(fp, "[DIR] %s/\n", item->name);
            export_btree_node(fp, item->data.directory->tree->root, depth + 1);
        }
    }
    if (!node->leaf) export_btree_node(fp, node->C[node->n], depth + 1);
}

void export_fs_to_img(Directory* root) {
    FILE* fp = fopen("fs.img", "w");
    if (!fp) { perror("Erro ao criar fs.img"); return; }
    fprintf(fp, "[ROOT]/\n");
    export_btree_node(fp, root->tree->root, 1);
    fclose(fp);
    printf("Sistema exportado para fs.img com sucesso.\n");
}

void mostrar_diretorio_atual(Directory* root, Directory* current) {
    char caminho[1024] = "";
    char temp[128];
    Directory* d = current;
    while (d != NULL && d != root) {
        BTreeNode* node = d->parent->tree->root;
        for (int i = 0; i < node->n; i++) {
            TreeNode* t = node->keys[i];
            if (t->type == DIRECTORY_TYPE && t->data.directory == d) {
                snprintf(temp, sizeof(temp), "/%s", t->name);
                strncat(temp, caminho, sizeof(temp) - strlen(temp) - 1);
                strncpy(caminho, temp, sizeof(caminho) - 1);
                break;
            }
        }
        d = d->parent;
    }
    if (strlen(caminho) == 0)
        printf("[ Diretório atual: / ]\n");
    else
        printf("[ Diretório atual: %s ]\n", caminho);
}

void listar_diretorios_recursivo(BTreeNode* node, int depth) {
    for (int i = 0; i < node->n; i++) {
        TreeNode* atual = node->keys[i];
        if (atual->type == DIRECTORY_TYPE) {
            for (int j = 0; j < depth; j++) printf("  ");
            printf("/%s\n", atual->name);
            listar_diretorios_recursivo(atual->data.directory->tree->root, depth + 1);
        }
        if (!node->leaf && node->C[i]) listar_diretorios_recursivo(node->C[i], depth);
    }
    if (!node->leaf && node->C[node->n]) listar_diretorios_recursivo(node->C[node->n], depth);
}

void listar_diretorios(Directory* dir) {
    printf("Diretórios acessíveis a partir daqui:\n");
    listar_diretorios_recursivo(dir->tree->root, 0);
}