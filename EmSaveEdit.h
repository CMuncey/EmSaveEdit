#ifndef EMSAVEEDIT_H
#define EMSAVEEDIT_H

#include <stdint.h>

#define NUM_SECS 14
#define SEC_SIZE 0xFF4

typedef struct
{
    uint8_t*  data;
    uint16_t secID;
    uint16_t check;
    uint32_t index;
}
sect_t;

typedef struct
{
    uint32_t personality;
    uint32_t otID;
    uint8_t  nick[10];
    uint16_t lang;
    uint8_t  OTname[7];
    uint8_t  markings;
    uint16_t checksum;
    uint16_t unknown;
    uint8_t  data[48];
    uint32_t status;
    uint8_t  level;
    uint8_t  pokerusTime;
    uint16_t curHP;
    uint16_t maxHP;
    uint16_t att;
    uint16_t def;
    uint16_t spe;
    uint16_t spA;
    uint16_t spD;
}
pkmn_t;

/* Growth */
typedef struct
{
    uint16_t species;
    uint16_t item;
    uint32_t exp;
    uint8_t  ppBonus;
    uint8_t  friends;
    uint16_t unknown;
}
pkdG_t;

/* Attacks */
typedef struct
{
    uint16_t move1;
    uint16_t move2;
    uint16_t move3;
    uint16_t move4;
    uint8_t  pp1;
    uint8_t  pp2;
    uint8_t  pp3;
    uint8_t  pp4;
}
pkdA_t;

/* EVs & Condition */
typedef struct
{
    uint8_t  hpEV;
    uint8_t  attEV;
    uint8_t  defEV;
    uint8_t  speEV;
    uint8_t  spAEV;
    uint8_t  spDEV;
    uint8_t  coolness;
    uint8_t  beauty;
    uint8_t  cuteness;
    uint8_t  smartness;
    uint8_t  toughness;
    uint8_t  feel;
}
pkdE_t;

/* Misc */
typedef struct
{
    uint8_t  pokerus;
    uint8_t  metLoc;
    uint16_t origin;
    uint32_t IVEA;
    uint32_t RibbonsObd;
}
pkdM_t;

typedef struct
{
    pkdG_t G;
    pkdA_t A;
    pkdE_t E;
    pkdM_t M;
}
pkdat_t;

void     toASCII(uint8_t*);
void     getPkdat(pkdat_t*, pkmn_t, uint32_t);
void     printPKMN(pkmn_t);
void     printParty(sect_t);
uint16_t getCheckPkmn(pkmn_t);
uint16_t getCheckSect(sect_t);

/* Order for G A E & M substructures in pkdat_t */
/* Numbers reperesent the position of GAEM */
/* Multiplied by 3 later to get correct offset */
uint8_t order[24][4] = {{0, 1, 2, 3},
                        {0, 1, 3, 2},
                        {0, 2, 1, 3},
                        {0, 3, 1, 2},
                        {0, 2, 3, 1},
                        {0, 3, 2, 1},
                        {1, 0, 2, 3},
                        {1, 0, 3, 2},
                        {2, 0, 1, 3},
                        {3, 0, 1, 2},
                        {2, 0, 3, 1},
                        {3, 0, 2, 1},
                        {1, 2, 0, 3},
                        {1, 3, 0, 2},
                        {2, 1, 0, 3},
                        {3, 1, 0, 2},
                        {2, 3, 0, 1},
                        {3, 2, 0, 1},
                        {1, 2, 3, 0},
                        {1, 3, 2, 0},
                        {2, 1, 3, 0},
                        {3, 1, 2, 0},
                        {2, 3, 1, 0},
                        {3, 2, 1, 0}};
#endif
