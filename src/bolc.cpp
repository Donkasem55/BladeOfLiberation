std::vector<std::vector<std::string>> lexer(std::string input) {
	std::string empty = "";
	std::vector<std::vector<std::string>> output = {{""}};
	int j = 0;
	int k = 0;
	for (int i=0; i<input.size(); i++) {
		char inp = input[i];
		if (inp == ' ' || inp == '\t') {
			if (output[j][k] != empty) {
				k += 1;
				output[j].push_back(empty);
			}
		}
		else if (inp == '\n') ;
		else if (inp == '(' || inp == ')' || inp == '{' || inp == '}') {
			if (output[j][k] != empty) {
				k += 1;
				output[j].push_back(empty);
			}
			output[j][k] += inp;
			k += 1;
			output[j].push_back(empty);
		}
		else if (inp == ';') {
			k = 0;
			j += 1;
			output.push_back({});
		}
		else {
			output[j][k] += inp;
		}
	}
	return output;
}

void parse(std::vector<std::vector<std::string>> input, std::vector<std::vector<std::string>> *triggers, unsigned long *password) {
	int scope = 0;
	bool bolc = false;
	for (int i=0; i<input.size(); i++) {
		if (input[i][0] == "#define") {
			if (input[i][1] == "pwd") *password = std::stoi(input[i][2]);
		} else if (input[i][0] == "#include") {
			if (input[i][1] == "<bolc>") bolc = true;
		} else if (bolc) {
			if (input[i][0] == "int" && input[i][1] == "trigger") {
				scope = 1;
			} else if (scope == 1) {
				if (input[i][0] == "}") {
					scope = 0;
				} else {
					(*triggers).push_back(input[i]);
				}
			}
		}
	}
}

void trigger() {}
