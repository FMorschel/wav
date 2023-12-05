#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windowsx.h>
#include <windows.h>
#pragma comment(lib, "winmm.lib")

/*
Você deverá manipular um arquivo de áudio (formato wave). 
Solicitar ao usuário para informar o nome do arquivo a ser manipulado.
Exibir as informações básicas do arquivo (taxa de amostragem, número de canais, etc).
O sistema desenvolvido deverá ter a opção de executar a música escolhida (opção padrão).
Também permitir ao usuário fazer um recorte na música, onde o usuário informa o tempo de início e fim do recorte a ser executado. Gerar um novo arquivo contendo o áudio recortado.

Informações  úteis:
Usar a biblioteca windows.h
Para executar o áudio usar a função PlaySound(  );
Inserir a biblioteca winmm dentro do codeblocks.
*/

// Trabalho WAV
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef uint8_t byte;
typedef uint16_t amostra_t;

// Cabecalho em portugues

typedef struct
{
    byte tipo[4];
    uint32_t tamanho;
    byte formato[4];
    byte subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t formatoAudio;
    uint16_t numCanais;
    uint32_t taxaAmostragem;
    uint32_t taxaBytes;
    uint16_t alinhamentoDeBlocos;
    uint16_t bitsPorAmostra;
    byte subchunk2ID[4];
    uint32_t subchunk2Size;
} cabecalho_t;

typedef struct
{
    uint32_t inicio;
    uint32_t fim;
    cabecalho_t cabecalho;
} dados_corte_t;

void escreverCabecalho(FILE *file, cabecalho_t cabecalho);
cabecalho_t cortarCabecalho(cabecalho_t cabecalho, dados_corte_t dados_corte);
uint32_t tempoEmSegundos(cabecalho_t cabecalho);
cabecalho_t lerCabecalho(FILE *file);
void printCabecalho(cabecalho_t cabecalho);

typedef enum
{
    tocando,
    parado,
} estado_musica_t;

int main()
{
    char nomeArquivo[100];
    char nomeArquivoSaida[106];
    char opcaoSelecionada[10];
    int opcao = 0;
    cabecalho_t cabecalho;
    dados_corte_t dados_corte;
    FILE *arquivo;
    arquivo = NULL;
    FILE *arquivoSaida;
    arquivoSaida = NULL;
    estado_musica_t estado_musica = parado;
    uint32_t segundos = 0;
    amostra_t amostra;

    while (1)
    {
        while (arquivo == NULL)
        {
            do
            {
                printf("Digite o nome do arquivo WAV: ");
                gets(nomeArquivo);
                // Se a extensao é .bmp ou se está sem extensao colocar e testar se encontra
                if ((strstr(nomeArquivo, ".wav") == NULL) && (strstr(nomeArquivo, ".WAV") == NULL))
                {
                    strcat(nomeArquivo, ".wav");
                }
                arquivo = fopen(nomeArquivo, "rb");
                if (arquivo == NULL)
                    printf("Arquivo nao encontrado, ou formato não WAV!\n");
            } while (arquivo == NULL);

            cabecalho = lerCabecalho(arquivo);

            if ((cabecalho.tipo[0] != 'R') || (cabecalho.tipo[1] != 'I') || (cabecalho.tipo[2] != 'F') || (cabecalho.tipo[3] != 'F'))
            {
                printf("Arquivo nao eh WAV!\n");
                printf("Tipo: %c%c%c%c\n", cabecalho.tipo[0], cabecalho.tipo[1], cabecalho.tipo[2], cabecalho.tipo[3]);
                fclose(arquivo);
                arquivo = NULL;
                strncpy(nomeArquivo, "\0", 100);
                continue;
            }

            // Testar por letras WAVE
            if ((cabecalho.formato[0] != 'W') || (cabecalho.formato[1] != 'A') || (cabecalho.formato[2] != 'V') || (cabecalho.formato[3] != 'E'))
            {
                printf("Arquivo nao eh WAV!\n");
                printf("Formato: %c%c%c%c\n", cabecalho.formato[0], cabecalho.formato[1], cabecalho.formato[2], cabecalho.formato[3]);
                fclose(arquivo);
                arquivo = NULL;
                strncpy(nomeArquivo, "\0", 100);
                continue;
            }

            // Testar por letras fmt
            if ((cabecalho.subchunk1ID[0] != 'f') || (cabecalho.subchunk1ID[1] != 'm') || (cabecalho.subchunk1ID[2] != 't') || (cabecalho.subchunk1ID[3] != ' '))
            {
                printf("Arquivo nao eh WAV!\n");
                printf("Subchunk1ID: %c%c%c%c\n", cabecalho.subchunk1ID[0], cabecalho.subchunk1ID[1], cabecalho.subchunk1ID[2], cabecalho.subchunk1ID[3]);
                fclose(arquivo);
                arquivo = NULL;
                strncpy(nomeArquivo, "\0", 100);
                continue;
            }

            // Testar por 16
            if (cabecalho.subchunk1Size != 16)
            {
                printf("Arquivo nao eh PCM!\n"); // PCM = Pulse Code Modulation (sem compressao)
                printf("Subchunk1Size: %d\n", cabecalho.subchunk1Size);
                fclose(arquivo);
                arquivo = NULL;
                strncpy(nomeArquivo, "\0", 100);
                continue;
            }

            // Testar por 1
            if (cabecalho.formatoAudio != 1) 
            {
                printf("Arquivo nao eh PCM!\n");
                printf("FormatoAudio: %d\n", cabecalho.formatoAudio);
                fclose(arquivo);
                arquivo = NULL;
                strncpy(nomeArquivo, "\0", 100);
                continue;
            }

            // Somente audios mono
            if (cabecalho.numCanais != 1)
            {
                printf("Arquivo nao eh mono!\n"); 
                printf("NumCanais: %d\n", cabecalho.numCanais);
                fclose(arquivo);
                arquivo = NULL;
                strncpy(nomeArquivo, "\0", 100);
                continue;
            }

            // Somente audios com taxa de amostragem de 44100
            if (cabecalho.taxaAmostragem != 44100)
            {
                printf("Arquivo nao eh 44100!\n");
                printf("TaxaAmostragem: %d\n", cabecalho.taxaAmostragem);
                fclose(arquivo);
                arquivo = NULL;
                strncpy(nomeArquivo, "\0", 100);
                continue;
            }

            // somente audios com bits por amostra de 16
            if (cabecalho.bitsPorAmostra != 16)
            {
                printf("Arquivo nao tem 16 bits por amostra!\n");
                printf("BitsPorAmostra: %d\n", cabecalho.bitsPorAmostra);
                fclose(arquivo);
                arquivo = NULL;
                strncpy(nomeArquivo, "\0", 100);
                continue;
            }

            // Testar por letras data
            if ((cabecalho.subchunk2ID[0] != 'd') || (cabecalho.subchunk2ID[1] != 'a') || (cabecalho.subchunk2ID[2] != 't') || (cabecalho.subchunk2ID[3] != 'a'))
            {
                printf("Arquivo nao eh WAV!\n");
                printf("Subchunk2ID: %c%c%c%c\n", cabecalho.subchunk2ID[0], cabecalho.subchunk2ID[1], cabecalho.subchunk2ID[2], cabecalho.subchunk2ID[3]);
                fclose(arquivo);
                arquivo = NULL;
                strncpy(nomeArquivo, "\0", 100);
                continue;
            }

            break;
        }

        if (arquivo == NULL)
            continue;
        if (strncmp(nomeArquivo, "\0", 100) == 0)
            continue;

        do
        {
            printf(
                "\nArqivo selecionado %s\n"
                "Menu\n"
                "1 - Tocar musica\n"
                "2 - Recortar\n"
                "3 - Mostrar cabecalho\n"
                "4 - Parar de reproduzir\n"
                "5 - Trocar de musica\n"
                "6 - Sair\n"
                "Digite a opcao desejada: ",
                nomeArquivo);
            gets(opcaoSelecionada);
            opcao = atoi(opcaoSelecionada); // atoi = ascii to integer
            if ((opcao < 1) || (opcao > 6))
                printf("Opcao invalida!\n");
        } while ((opcao < 1) || (opcao > 6));

        if (opcao == 1)
        {
            if (estado_musica == tocando)
            {
                PlaySound(NULL, NULL, SND_FILENAME | SND_ASYNC);
            }
            PlaySound(nomeArquivo, NULL, SND_FILENAME | SND_ASYNC);
            estado_musica = tocando;
        }
        else if (opcao == 2)
        {

            segundos = tempoEmSegundos(cabecalho);

            printf("Tempo total da musica: %d segundos\n", segundos);

            do
            {
                printf("Digite o tempo de inicio do recorte em segundos (0s): ");
                gets(opcaoSelecionada);
                opcao = atoi(opcaoSelecionada); // atoi = ascii to integer
                if ((opcao < 0) || (opcao > segundos))
                    printf("Opcao invalida!\n");
            } while ((opcao < 0) || (opcao > segundos));

            dados_corte.inicio = opcao;

            do
            {
                printf("Digite o tempo de fim do recorte em segundos: ");
                gets(opcaoSelecionada);
                opcao = atoi(opcaoSelecionada); // atoi = ascii to integer
                if ((opcao <= dados_corte.inicio) || (opcao > segundos))
                    printf("Opcao invalida!\n");
            } while ((opcao <= dados_corte.inicio) || (opcao > segundos));

            dados_corte.fim = opcao;

            do
            {
                printf("Digite o nome do arquivo de saida: ");
                gets(nomeArquivoSaida);
                // Se a extensao é .wav ou se está sem extensao colocar e testar se encontra
                if (strlen(nomeArquivoSaida) == 0)
                    continue;
                if ((strstr(nomeArquivoSaida, ".wav") == NULL) && (strstr(nomeArquivoSaida, ".WAV") == NULL))
                {
                    strcat(nomeArquivoSaida, ".wav");
                }
                arquivoSaida = fopen(nomeArquivoSaida, "wb");
                if (arquivoSaida == NULL)
                    printf("Arquivo nao encontrado, ou formato não WAV!\n");
            } while (arquivoSaida == NULL);

            dados_corte.cabecalho = cortarCabecalho(cabecalho, dados_corte);

            escreverCabecalho(arquivoSaida, dados_corte.cabecalho);

            fseek(arquivo, 44 + dados_corte.inicio * cabecalho.taxaBytes, SEEK_SET);

            for (int i = 0; i < ((dados_corte.fim - dados_corte.inicio) * dados_corte.cabecalho.taxaAmostragem); i++)
            {
                fread(&amostra, 2, 1, arquivo);
                fwrite(&amostra, 2, 1, arquivoSaida);
            }

            fclose(arquivoSaida);
        }
        else if (opcao == 3)
        {
            printCabecalho(cabecalho);
        }
        else if (opcao == 4)
        {
            estado_musica = parado;
            PlaySound(NULL, NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (opcao == 5)
        {
            estado_musica = parado;
            PlaySound(NULL, NULL, SND_FILENAME | SND_ASYNC);
            puts(""); // printf("\n");
            fclose(arquivo);
            arquivo = NULL;
            strncpy(nomeArquivo, "\0", 100);
            continue;
        }
        else if (opcao == 6)
        {
            estado_musica = parado;
            PlaySound(NULL, NULL, SND_FILENAME | SND_ASYNC);
            fclose(arquivo);
            arquivo = NULL;
            strncpy(nomeArquivo, "\0", 100);
            break;
        }
    }

    return 0;
}

void escreverCabecalho(FILE *file, cabecalho_t cabecalho)
{
    fseek(file, 0, SEEK_SET);
    fwrite(&cabecalho.tipo[0], 1, 1, file);
    fwrite(&cabecalho.tipo[1], 1, 1, file);
    fwrite(&cabecalho.tipo[2], 1, 1, file);
    fwrite(&cabecalho.tipo[3], 1, 1, file);
    fwrite(&cabecalho.tamanho, 4, 1, file);
    fwrite(&cabecalho.formato[0], 1, 1, file);
    fwrite(&cabecalho.formato[1], 1, 1, file);
    fwrite(&cabecalho.formato[2], 1, 1, file);
    fwrite(&cabecalho.formato[3], 1, 1, file);
    fwrite(&cabecalho.subchunk1ID[0], 1, 1, file);
    fwrite(&cabecalho.subchunk1ID[1], 1, 1, file);
    fwrite(&cabecalho.subchunk1ID[2], 1, 1, file);
    fwrite(&cabecalho.subchunk1ID[3], 1, 1, file);
    fwrite(&cabecalho.subchunk1Size, 4, 1, file);
    fwrite(&cabecalho.formatoAudio, 2, 1, file);
    fwrite(&cabecalho.numCanais, 2, 1, file);
    fwrite(&cabecalho.taxaAmostragem, 4, 1, file);
    fwrite(&cabecalho.taxaBytes, 4, 1, file);
    fwrite(&cabecalho.alinhamentoDeBlocos, 2, 1, file);
    fwrite(&cabecalho.bitsPorAmostra, 2, 1, file);
    fwrite(&cabecalho.subchunk2ID[0], 1, 1, file);
    fwrite(&cabecalho.subchunk2ID[1], 1, 1, file);
    fwrite(&cabecalho.subchunk2ID[2], 1, 1, file);
    fwrite(&cabecalho.subchunk2ID[3], 1, 1, file);
    fwrite(&cabecalho.subchunk2Size, 4, 1, file);
}

cabecalho_t lerCabecalho(FILE *file)
{
    cabecalho_t cabecalho;
    fseek(file, 0, SEEK_SET);
    fread(&cabecalho.tipo[0], 1, 1, file);
    fread(&cabecalho.tipo[1], 1, 1, file);
    fread(&cabecalho.tipo[2], 1, 1, file);
    fread(&cabecalho.tipo[3], 1, 1, file);
    fread(&cabecalho.tamanho, 4, 1, file);
    fread(&cabecalho.formato[0], 1, 1, file);
    fread(&cabecalho.formato[1], 1, 1, file);
    fread(&cabecalho.formato[2], 1, 1, file);
    fread(&cabecalho.formato[3], 1, 1, file);
    fread(&cabecalho.subchunk1ID[0], 1, 1, file);
    fread(&cabecalho.subchunk1ID[1], 1, 1, file);
    fread(&cabecalho.subchunk1ID[2], 1, 1, file);
    fread(&cabecalho.subchunk1ID[3], 1, 1, file);
    fread(&cabecalho.subchunk1Size, 4, 1, file);
    fread(&cabecalho.formatoAudio, 2, 1, file);
    fread(&cabecalho.numCanais, 2, 1, file);
    fread(&cabecalho.taxaAmostragem, 4, 1, file);
    fread(&cabecalho.taxaBytes, 4, 1, file);
    fread(&cabecalho.alinhamentoDeBlocos, 2, 1, file);
    fread(&cabecalho.bitsPorAmostra, 2, 1, file);
    fread(&cabecalho.subchunk2ID[0], 1, 1, file);
    fread(&cabecalho.subchunk2ID[1], 1, 1, file);
    fread(&cabecalho.subchunk2ID[2], 1, 1, file);
    fread(&cabecalho.subchunk2ID[3], 1, 1, file);
    fread(&cabecalho.subchunk2Size, 4, 1, file);
    return cabecalho;
}

void printCabecalho(cabecalho_t cabecalho)
{
    printf("Tipo: %c%c%c%c\n", cabecalho.tipo[0], cabecalho.tipo[1], cabecalho.tipo[2], cabecalho.tipo[3]);
    printf("Tamanho: %d\n", cabecalho.tamanho);
    printf("Formato: %c%c%c%c\n", cabecalho.formato[0], cabecalho.formato[1], cabecalho.formato[2], cabecalho.formato[3]);
    printf("Subchunk1ID: %c%c%c%c\n", cabecalho.subchunk1ID[0], cabecalho.subchunk1ID[1], cabecalho.subchunk1ID[2], cabecalho.subchunk1ID[3]);
    printf("Subchunk1Size: %d\n", cabecalho.subchunk1Size);
    printf("FormatoAudio: %d\n", cabecalho.formatoAudio);
    printf("NumCanais: %d\n", cabecalho.numCanais);
    printf("TaxaAmostragem: %d\n", cabecalho.taxaAmostragem);
    printf("TaxaBytes: %d\n", cabecalho.taxaBytes);
    printf("AlinhamentoDeBlocos: %d\n", cabecalho.alinhamentoDeBlocos);
    printf("BitsPorAmostra: %d\n", cabecalho.bitsPorAmostra);
    printf("Subchunk2ID: %c%c%c%c\n", cabecalho.subchunk2ID[0], cabecalho.subchunk2ID[1], cabecalho.subchunk2ID[2], cabecalho.subchunk2ID[3]);
    printf("Subchunk2Size: %d\n", cabecalho.subchunk2Size);
}

uint32_t tempoEmSegundos(cabecalho_t cabecalho)
{
    return cabecalho.subchunk2Size / cabecalho.taxaBytes;
}

cabecalho_t cortarCabecalho(cabecalho_t cabecalho, dados_corte_t dados_corte)
{
    cabecalho_t cabecalho_cortado;
    cabecalho_cortado = cabecalho;

    int durationInSeconds = dados_corte.fim - dados_corte.inicio;
    int numSamples = durationInSeconds * cabecalho.taxaAmostragem;
    int numBytes = numSamples * cabecalho.bitsPorAmostra / 8;

    cabecalho_cortado.subchunk2Size = numBytes;
    cabecalho_cortado.tamanho = cabecalho_cortado.subchunk2Size + 36;

    return cabecalho_cortado;
}
