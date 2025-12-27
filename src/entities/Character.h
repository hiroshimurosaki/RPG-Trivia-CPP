#pragma once 
#include <SFML/Graphics.hpp> 

class Character {
public:
    sf::Sprite bodySprite;
    sf::Sprite helmetSprite;
    sf::Sprite weaponSprite;
    
    sf::Vector2f headOffset; 
    sf::Vector2f handOffset; 

    void Setup(std::string type) {
        if (type == "Frog") {
            headOffset = {0, -40};
            handOffset = {50, 30}; 
        }
    }

    void EquipItem(std::string itemType, sf::Texture& texture) {
        if (itemType == "Helmet") {
            helmetSprite.setTexture(texture);
        }
    }

    void Draw(sf::RenderWindow& window) {
        // 1. Desenha o corpo
        window.draw(bodySprite);

        // 2. Atualiza e desenha o capacete
        if (helmetSprite.getTexture()) {
            // Pega a posição ATUAL do corpo e soma o offset
            helmetSprite.setPosition(bodySprite.getPosition() + headOffset);
            window.draw(helmetSprite);
        }

        // 3. Atualiza e desenha a arma
        if (weaponSprite.getTexture()) {
            weaponSprite.setPosition(bodySprite.getPosition() + handOffset);
            window.draw(weaponSprite);
        }
    }
};