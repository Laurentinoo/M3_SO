#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"

void esperar_para_continuar() {
    printf("\nPressione Enter para continuar...");
    while (getchar() != '\n');
    getchar();
}

void limpar_tela() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void mostrar_menu() {
    printf("============================================\n");
    printf("           SISTEMA DE ARQUIVOS\n");
    printf("============================================\n");
    printf("Comandos disponíveis:\n");
    printf("  mkdir <nome>          (cria um diretório)\n");
    printf("  rmdir <nome>          (remove um diretório vazio)\n");
    printf("  touch <nome> <texto>  (cria um arquivo .txt)\n");
    printf("  rm <nome>             (remove um arquivo .txt)\n");
    printf("  cd <nome>             (entra em um subdiretório / 'cd ..' para voltar)\n");
    printf("  cs list               (lista todos os diretórios possíveis)\n");
    printf("  ls                    (lista o conteúdo do diretório atual)\n");
    printf("  export                (gera imagem textual do sistema em fs.img)\n");
    printf("  exit                  (sair do sistema)\n");
    printf("============================================\n");
}

int main() {
    Directory* root = get_root_directory(); // Cria o diretório raiz
    Directory* current = root; // Diretório atual (navegação)

    char command[256];
    char arg1[128], arg2[1024];

    while (1) {
    limpar_tela();         // limpa a tela antes de mostrar o menu
    mostrar_diretorio_atual(root, current); // mostra o diretório vigente
    mostrar_menu();        // mostra o menu durante toda a execução

    printf(">> ");
    fgets(command, sizeof(command), stdin);
    command[strcspn(command, "\n")] = 0;

    if (strncmp(command, "exit", 4) == 0) {
        break;
    } else if (sscanf(command, "mkdir %s", arg1) == 1) {
        TreeNode* dir = create_directory(arg1, current);
        btree_insert(current->tree, dir);
    } else if (sscanf(command, "rmdir %s", arg1) == 1) {
        delete_directory(current->tree, arg1);
    } else if (sscanf(command, "touch %s %[^\n]", arg1, arg2) == 2) {
        TreeNode* file = create_txt_file(arg1, arg2);
        btree_insert(current->tree, file);
    } else if (sscanf(command, "rm %s", arg1) == 1) {
        delete_txt_file(current->tree, arg1);
    } else if (sscanf(command, "cd %s", arg1) == 1) {
        change_directory(&current, arg1);
    } else if (strcmp(command, "ls") == 0) {
        list_directory_contents(current);
    } else if (strcmp(command, "export") == 0) {
        export_fs_to_img(root);
    } else if (strcmp(command, "cs list") == 0) {
        listar_diretorios(current);
    } else {
        printf("Comando inválido.\n");
    }

    esperar_para_continuar();
}

    return 0;
}