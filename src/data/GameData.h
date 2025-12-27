#pragma once
#include <string>
#include <vector>

enum class GameState {
    LanguageSelect, MainMenu, Debug, ClassSelect, Playing, Store
};

struct RpgClass {
    std::string name;
    std::string description;

    std::vector<std::string> expertises; 
};

inline std::vector<RpgClass> LoadClasses() {
    return {

        {"Engenheiro", "Mestre em calculos e leis da fisica.", 
         {"Matematica", "Fisica", "Quimica"}},

        {"Cinefilo", "Viciado em telas e cultura pop.", 
         {"Entretenimento"}}, 

        {"Biologo", "Conhecedor da vida e natureza.", 
         {"Biologia", "Quimica"}},

        {"Explorador", "Viajante do mundo e do tempo.", 
         {"Historia", "Geografia"}},

        {"Esportista", "Atleta de corpo e mente.", 
         {"Esportes"}},

        {"Musicista", "Ouvido absoluto.", 
         {"Entretenimento", "Variedades"}}, 

        {"Nerd", "Curioso sobre tudo.", 
         {"Variedades", "Matematica", "Fisica", "Historia", "Tecnologia"}} 
    };
}