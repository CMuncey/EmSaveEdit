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

int main(int argc, char** argv)
{
    int i;
    FILE* file;
    uint32_t key, FID, psize;
    uint16_t TID, SID;
    sect_t tempSect;
    sect_t* sects;
    pkmn_t* party;
    pkdat_t* tempDat;

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

    /* Key and Trainer IDs */
    FID = *((uint32_t*)(sects[0].data + 0x0A));
    TID = FID & 0xFFFF;
    SID = (FID >> 16) & 0xFFFF;
    key = *((uint32_t*)(sects[0].data + 0x01F4));

    //for(i = 0; sects[0].data[i] != 0xFF; i++)
    //    printf("%02x ", sects[0].data[i]);
    //printf("%02x\n", sects[0].data[i]);
    toASCII(sects[0].data);
    printf("%s\n", sects[0].data);
    printf("TID: %d | SID: %d | key: %#x\n\n", TID, SID, key);
    tempDat = malloc(sizeof(pkdat_t));

    /* Read party, print out data */
    psize = *((uint32_t*)(sects[1].data + 0x0234));
    party = (pkmn_t*)(sects[1].data + 0x0238);
    for(i = 0; i < psize; i++)
    {
        toASCII(party[i].nick);
        getPkdat(tempDat, party[i], FID);
        printf("%sLV %d %s [%s] (%d exp):%s\n", CYAN, party[i].level, names[tempDat->G.species], party[i].nick, tempDat->G.exp, WHITE);
        printf("  HP:   %3d [%2d | %2d]\n", party[i].maxHP, tempDat->E.hpEV,  (tempDat->M.IVEA)       & 0x1F);
        printf("  ATT:  %3d [%2d | %2d]\n", party[i].att,   tempDat->E.attEV, (tempDat->M.IVEA >>  5) & 0x1F);
        printf("  DEF:  %3d [%2d | %2d]\n", party[i].def,   tempDat->E.defEV, (tempDat->M.IVEA >> 10) & 0x1F);
        printf("  SPA:  %3d [%2d | %2d]\n", party[i].spA,   tempDat->E.spAEV, (tempDat->M.IVEA >> 20) & 0x1F);
        printf("  SPD:  %3d [%2d | %2d]\n", party[i].spD,   tempDat->E.spDEV, (tempDat->M.IVEA >> 25) & 0x1F);
        printf("  SPE:  %3d [%2d | %2d]\n", party[i].spe,   tempDat->E.speEV, (tempDat->M.IVEA >> 15) & 0x1F);
        printf("\n");
    }

    return(0);
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
