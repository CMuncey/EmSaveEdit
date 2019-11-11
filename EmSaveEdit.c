#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "EmSaveEdit.h"
#include "pokemon_list.h"

/* Colors for printf */
#define WHITE   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"

uint32_t key, FID;
uint16_t TID, SID;

int main(int argc, char** argv)
{
    int i;
    FILE* file;
    char str[32];
    sect_t tempSect;
    sect_t* sects;

    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s [input file]\n", argv[0]);
        exit(1);
    }

    /* Open file, allocate memory for sectors */
    file = fopen(argv[1], "rb");
    sects = malloc(sizeof(sect_t) * NUM_SECS);
    
    /* Read each sector */
    /* There's 4 bytes of padding after the checksum */
    for(i = 0; i < NUM_SECS; i++)
    {
        tempSect.data = malloc(SEC_SIZE);
        fread(tempSect.data, 1, SEC_SIZE, file);
        fread(&(tempSect.secID), 2, 1, file);
        fread(&(tempSect.check), 2, 1, file);
        fread(&(tempSect.index), 4, 1, file);
        fread(&(tempSect.index), 4, 1, file);
        sects[tempSect.secID] = tempSect;
    }
    fclose(file);

    /* Key and Trainer IDs */
    FID = *((uint32_t*)(sects[0].data + 0x0A));
    TID = FID & 0xFFFF;
    SID = (FID >> 16) & 0xFFFF;
    key = *((uint32_t*)(sects[0].data + 0x01F4));
    
    /* Print out trainer name and keys */
    memcpy(str, sects[0].data, 8);
    toASCII(str);
    printf("%sTrainer name: %s%s\n", CYAN, WHITE, str);
    printf("%sTID: %s%d\n", CYAN, WHITE, TID);
    printf("%sSID: %s%d\n", CYAN, WHITE, SID);
    printf("%skey: %s0x%08X\n\n", CYAN, WHITE, key);

    do
    {
        printf("> ");
        if(fgets(str, 32, stdin) == NULL)
        {
            printf("\n");
            break;
        }

        str[strlen(str)-1] = '\0';

        if(strncmp(str, "quit", 5) == 0)
            break;

        if(strncmp(str, "p party", 6) == 0)
        {
            printParty(sects[1]);
            continue;
        }
    }
    while(1);

    free(sects);

    return(0);
}

void printParty(sect_t sect)
{
    uint32_t pSize, i;
    pkmn_t* party;

    pSize = *((uint32_t*)(sect.data + 0x0234));
    party = (pkmn_t*)(sect.data + 0x0238);
    for(i = 0; i < pSize; i++)
    {
        printf("\n");
        printPKMN(party[i]);
        printf("\n");
    }
}

void printPKMN(pkmn_t p)
{
    char name[10];
    pkdat_t dat;

    memcpy(name, p.nick, 10);
    toASCII(name);
    getPkdat(&dat, p, FID);
    
    printf("%sLV %d %s [%s] (%d exp):%s\n", CYAN, p.level, names[dat.G.species], name, dat.G.exp, WHITE);
    printf("  HP:   %3d [%2d | %2d]\n", p.maxHP, dat.E.hpEV,  (dat.M.IVEA)       & 0x1F);
    printf("  ATT:  %3d [%2d | %2d]\n", p.att,   dat.E.attEV, (dat.M.IVEA >>  5) & 0x1F);
    printf("  DEF:  %3d [%2d | %2d]\n", p.def,   dat.E.defEV, (dat.M.IVEA >> 10) & 0x1F);
    printf("  SPA:  %3d [%2d | %2d]\n", p.spA,   dat.E.spAEV, (dat.M.IVEA >> 20) & 0x1F);
    printf("  SPD:  %3d [%2d | %2d]\n", p.spD,   dat.E.spDEV, (dat.M.IVEA >> 25) & 0x1F);
    printf("  SPE:  %3d [%2d | %2d]\n", p.spe,   dat.E.speEV, (dat.M.IVEA >> 15) & 0x1F);
}

void toASCII(uint8_t* str)
{
    int i;
    
    for(i = 0; str[i] != 0xFF; i++)
    {
        if(str[i] == 0x00)
            str[i] = ' ';
        else if(str[i] == 0xAB)
            str[i] = '!';
        else if(str[i] == 0xAC)
            str[i] = '?';
        else if(str[i] == 0xAD)
            str[i] = '.';
        else if(str[i] == 0xAE)
            str[i] = '-';
        else if(str[i] >= 0xA1 && str[i] <= 0xAA)
            str[i] -= 0x71;
        else if(str[i] >= 0xBB && str[i] <= 0xD4)
            str[i] -= 0x7A;
        else if(str[i] >= 0xD5 && str[i] <= 0xEE)
            str[i] -= 0x74;
        else
            str[i] = 0x80;
    }
    str[i] = '\0';
}

void getPkdat(pkdat_t* pkdat, pkmn_t pkmn, uint32_t FID)
{
    uint32_t i, key, pval;
    uint32_t* temp;
    
    if(pkdat == NULL)
    {
        fprintf(stderr, "Error: NULL pkdat in getPkdat\n");
        exit(1);
    }

    pval = pkmn.personality;
    key = pval ^ FID;
    pval %= 24;
    temp = malloc(sizeof(uint32_t) * 12);
    memcpy(temp, pkmn.data, 48);

    /* Decrypt the data */
    for(i = 0; i < 12; i++)
        temp[i] ^= key;

    /* Get G A E & M data from temp */
    memcpy(&(pkdat->G), temp + (order[pval][0] * 3), 12);
    memcpy(&(pkdat->A), temp + (order[pval][1] * 3), 12);
    memcpy(&(pkdat->E), temp + (order[pval][2] * 3), 12);
    memcpy(&(pkdat->M), temp + (order[pval][3] * 3), 12); 
    free(temp);
}
