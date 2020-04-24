//
// Created by utnso on 18/04/20.
//

#ifndef TP_2020_1C_RECURSANTES_3_0_STRUCTURES_H
#define TP_2020_1C_RECURSANTES_3_0_STRUCTURES_H
#endif //TP_2020_1C_RECURSANTES_3_0_STRUCTURES_H

enum PROCESO{
    BROKER,
    TEAM,
    GAMECARD,
    SUSCRIPTOR
};

enum mensajes{
        NEW_POKEMON,
        APPEARED_POKEMON,
        CATCH_POKEMON,
        CAUGHT_POKEMON,
        GET_POKEMON,
        LOCALIZED_POKEMON
};

const static struct {
    u_int32_t    val;
    const char *str;
} conversionProces [] = {
        {0, "BROKER"},
        {1, "TEAM"},
        {2, "GAMECARD"},
        {3, "SUSCRIPTOR"}
};

const static struct {
    u_int32_t      val;
    const char *str;
} conversionMsj [] = {
        {0, "NEW_POKEMON"},
        {1, "APPEARED_POKEMON"},
        {2, "CATCH_POKEMON"},
        {3, "CAUGHT_POKEMON"},
        {4, "GET_POKEMON"},
        {5, "LOCALIZED_POKEMON"}
};


