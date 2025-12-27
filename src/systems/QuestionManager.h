#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

enum class QuestionType { MultipleChoice, TextConfig, ImageInput };

struct Question {
    int id;
    QuestionType type;
    std::string category; 
    std::string subtheme; 
    std::string text;
    std::string imagePath; 
    std::vector<std::string> options; 
    int correctIndex; 
    std::vector<std::string> validAnswers; 
};

class QuestionManager {
public:
    std::vector<Question> questions;

    void LoadAllCategories() {
        std::vector<std::string> files = {
            "assets/json/matematica.json",
            "assets/json/fisica.json",
            "assets/json/biologia.json",
            "assets/json/quimica.json",
            "assets/json/geografia.json",
            "assets/json/historia.json",
            "assets/json/esportes.json",
            "assets/json/entretenimento.json",
            "assets/json/variedades.json"
        };

        for (const auto& file : files) {
            LoadQuestionsFromFile(file);
        }
    }

    void LoadQuestionsFromFile(const std::string& filename) {
        std::ifstream f(filename);
        if (!f.is_open()) return; 

        json data = json::parse(f);
        // Pega a categoria do arquivo (ex: "Matematica")
        std::string fileCategory = data.value("category", "Geral");

        for (auto& q : data["questions"]) {
            Question newQ;

            newQ.id = q.value("id", 0);
            newQ.text = q.value("text", "");
            newQ.category = fileCategory;
            newQ.subtheme = q.value("subtheme", "Geral");

            std::string typeStr = q.value("type", "multiple_choice");
            if (typeStr == "multiple_choice") newQ.type = QuestionType::MultipleChoice;
            else if (typeStr == "text_input") newQ.type = QuestionType::TextConfig;
            else newQ.type = QuestionType::ImageInput;

            if (q.contains("options")) {
                newQ.options = q["options"].get<std::vector<std::string>>();
                newQ.correctIndex = q.value("correct_index", 0);
            }

            if (q.contains("valid_answers")) {
                newQ.validAnswers = q["valid_answers"].get<std::vector<std::string>>();
            }

            questions.push_back(newQ);
        }
    }

    Question GetRandomQuestion() {
        if (questions.empty()) return {}; 
        int idx = rand() % questions.size();
        return questions[idx];
    }
};