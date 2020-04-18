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

const static struct {
    u_int32_t    val;
    const char *str;
} conversionProces [] = {
        {1, "BROKER"},
        {2, "TEAM"},
        {3, "GAMECARD"},
        {4, "SUSCRIPTOR"}
};

const static struct {
    u_int32_t      val;
    const char *str;
} conversionMsj [] = {
        {1, "NEW_POKEMON"},
        {2, "APPEARED_POKEMON"},
        {3, "CATCH_POKEMON"},
        {4, "CAUGHT_POKEMON"},
        {5, "GET_POKEMON"},
        {6, "LOCALIZED_POKEMON"}
};