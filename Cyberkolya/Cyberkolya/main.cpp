#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <string>
#include <sstream>
#include <stdexcept>
#include "cyberkolya.h"
#include "legacy_database.h"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>

int main() {
	_setmode(_fileno(stdout), _O_U16TEXT);
	LegacyDatabase database;
	Cyberkolya cyberkolya(database);
	cyberkolya.read_from_files("../database/users.tsv",
		{"../database/inside.tsv", "../database/teamtardis.tsv", "../database/insidenot.tsv", 
		"../database/sfedu.tsv",	"../database/mmcs.tsv" });

	std::wstringstream cmd(LR"(
		insert_user 1 Александр Иванов
		insert_user 2 Петр		Дуров
		insert_user 3 Павел		Сидоров
		insert_post 1 123 3 1714676192 0 0
		insert_post 2 123 2 1714676193 0 0
		insert_post 1 124 1 1714676194 0 0
		like_post 123 1
		like_post 123 1
		like_post 123 1
		like_post 123 1
		like_post 123 2
		like_post 124 1
		like_post 124 1
		get_user 1
		get_user 2
		get_user 3
		top_k_posts_by_likes 10 123 1704056400 1735678800
		top_k_authors_by_likes 10 123 1704056400 1735678800
		top_k_posts_by_likes 10 -88704999 1704056400 1735678800
		top_k_authors_by_likes 10 -88704999 1704056400 1735678800
		help)");
	while (!cmd.eof()) {
		cyberkolya.process_event(cmd, std::wcout);
	}
	while (!std::wcin.eof() && std::wcin.good()) {
		std::wcout << "admin$: ";
		cyberkolya.process_event(std::wcin, std::wcout);
	}
}