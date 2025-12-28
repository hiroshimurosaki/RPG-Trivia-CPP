#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>

// Tipos de Slots de Equipamento
enum class EquipSlot { Hat, Mask, Suit, Weapon, Acc1, Acc2 };

// Tipos de Buff (Exemplo)
enum class BuffType { Poison, Shield, Strength, Weakness };

struct Buff {
    std::string name;
    BuffType type;
    int turnsRemaining;
    bool isDebuff; // True = Vermelho, False = Verde
};

class Character {
public:
    std::string name;
    int maxHp;
    int currentHp;
    
    // Vetor de Buffs
    std::vector<Buff> buffs;

    // Visual: Corpo Base
    sf::Sprite bodySprite;
    sf::Color baseColor;

    // Visual: Equipamentos (Mapeia o Slot -> Sprite)
    std::map<EquipSlot, sf::Sprite> equipmentSprites;

    // Offsets (Ajuste fino de onde cada item fica no corpo)
    std::map<EquipSlot, sf::Vector2f> itemOffsets;

    Character() : maxHp(100), currentHp(100), baseColor(sf::Color::White) {}

    void Setup(std::string _name, int _hp, sf::Color color) {
        name = _name;
        maxHp = _hp;
        currentHp = _hp;
        baseColor = color;
        bodySprite.setColor(color);
        
        // Configura offsets padrão (ajustaremos isso visualmente depois)
        // Exemplo: Chapéu fica 20px acima do centro, Arma 40px pra direita
        itemOffsets[EquipSlot::Hat] = {0, -50};
        itemOffsets[EquipSlot::Mask] = {0, -10};
        itemOffsets[EquipSlot::Suit] = {0, 0};
        itemOffsets[EquipSlot::Weapon] = {40, 10};
        itemOffsets[EquipSlot::Acc1] = {-40, 10};
        itemOffsets[EquipSlot::Acc2] = {40, -40};
    }

    // Função para equipar visualmente
    void EquipItem(EquipSlot slot, sf::Texture& texture) {
        sf::Sprite s;
        s.setTexture(texture);
        // Centraliza a origem do item para facilitar o posicionamento
        sf::Vector2u size = texture.getSize();
        s.setOrigin(size.x / 2.0f, size.y / 2.0f);
        equipmentSprites[slot] = s;
    }

    void AddBuff(std::string bName, BuffType bType, int turns, bool isBad) {
        buffs.push_back({bName, bType, turns, isBad});
    }

    void TakeDamage(int dmg) {
        currentHp -= dmg;
        if (currentHp < 0) currentHp = 0;
    }

    // Desenha o personagem e todos os seus itens na posição correta
    void Draw(sf::RenderWindow& window, sf::Vector2f position, float scale) {
        // 1. Configura posição e escala do corpo
        bodySprite.setPosition(position);
        bodySprite.setScale(scale, scale);
        window.draw(bodySprite);

        // 2. Desenha cada item equipado
        for (auto& [slot, sprite] : equipmentSprites) {
            // Calcula a posição do item: Posição do Corpo + (Offset * Escala)
            sf::Vector2f finalPos = position + (itemOffsets[slot] * scale);
            
            sprite.setPosition(finalPos);
            sprite.setScale(scale, scale);
            window.draw(sprite);
        }
    }
};