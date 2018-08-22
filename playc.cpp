#include "cbb.h"
#include <iostream>
#include <fstream>

int main() {
	std::string input;

	std::cout << "Will player #0 be the computer? (y/N): "; 
	std::getline(std::cin,input);
	bool player0c = std::tolower(input[0]) == 'y';

	std::cout << "Will player #1 be the computer? (y/N): ";
	std::getline(std::cin,input);
	bool player1c = std::tolower(input[0]) == 'y';

	std::string board = "";
	int player = 0;
	int timeLimit = 0;
	while (board == "") {
		std::cout << "Load a game from a file (press enter for default): ";
		std::getline(std::cin,input);
		if (input != "") {
			std::ifstream file(input);
			int n;
			if (file.is_open()) {
				for (int i = 0; i < 32; i++) {
					file >> n;
					board += '0' + n;
				}
				file >> player;
				player--;
				file >> timeLimit;
			} else {
				std::cout << "File '" << input << "' not found!\n";
			}
		} else {
			board = "22222222222200000000111111111111";
		}
	}

	if (player0c || player1c) {
		while (timeLimit < 1) {
			std::cout << "Enter a time limit for the computer to make a move (sec): ";
			std::getline(std::cin,input);
			try {
				timeLimit = std::stoi(input);
			} catch (std::exception &) {
				std::cout << "Invalid time limit '" << input << "' !\n";
			}
		}
	}

	cbb game = cbb(board.c_str(),player,timeLimit);
	std::cout << "\n";
	game.printcb();
	int pick = 0;
	int gamewon = -1;
	while (gamewon < 0) {
		time_t time = 0;
		if ((player0c && game.getPlayer() == 0) || (player1c && game.getPlayer() == 1)) {
			if((time = game.aiPickMove()) >= 0) {
				std::cout << "\nPlayer " << !game.getPlayer() << " took " << time << " seconds to make its move.\n";
				game.printcb();
			} else {
				gamewon = !game.getPlayer();
				break;
			}
		} else {
			if (game.printlms() == 0) {
				gamewon = !game.getPlayer();
				break;
			}
			bool valid;
			do {
				std::cout << "\nPlayer " << game.getPlayer() << ", pick a move: #";
				std::getline(std::cin,input);
				try {
					pick = std::stoi(input);
					valid = game.humanPickMove(pick);
				} catch (std::exception &) {
					valid = false;
				}
				if (valid) {
					game.printcb();
				} else {
					std::cout << "That move is not in the list.";
				}
			} while (!valid);
		}
	}
	std::cout << "\nPlayer " << gamewon << " Wins!\n***Game Over***\n";
}
