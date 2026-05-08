#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {

constexpr int kWordLength = 5;
constexpr int kMaxTries = 6;
const char *kAllWordsFile = "ALL.TXT";
const char *kSolutionFile = "SOLUTION.TXT";
const char *kStatsFile = ".wordle_stats";

const char *kGreen = "\033[48;5;2m\033[30m";
const char *kYellow = "\033[48;5;3m\033[30m";
const char *kGray = "\033[48;5;240m\033[37m";
const char *kRed = "\033[48;5;1m\033[37m";
const char *kReset = "\033[0m";

struct Dictionary {
    std::vector<std::string> solutions;
    std::unordered_set<std::string> allowed;
};

struct GuessResult {
    std::array<int, kWordLength> marks{}; // 0 gray, 1 yellow, 2 green
};

struct GameOptions {
    bool hardMode = false;
    bool dailyMode = false;
    bool quietAnswer = false;
};

struct GameResult {
    bool won = false;
    int attempts = 0;
    std::string answer;
    std::vector<std::string> guesses;
    std::vector<GuessResult> results;
    bool quietAnswer = false;
};

struct Stats {
    int played = 0;
    int wins = 0;
    int currentStreak = 0;
    int bestStreak = 0;
    int distribution[kMaxTries + 1]{};
};

std::string trimAndLower(std::string word) {
    std::string out;
    for (unsigned char ch : word) {
        if (std::isalpha(ch)) {
            out.push_back(static_cast<char>(std::tolower(ch)));
        }
    }
    return out;
}

std::vector<std::string> readWords(const std::string &path) {
    std::ifstream input(path);
    std::vector<std::string> words;
    std::string line;
    while (std::getline(input, line)) {
        std::string word = trimAndLower(line);
        if (word.size() == kWordLength) {
            words.push_back(word);
        }
    }
    return words;
}

Dictionary loadDictionary() {
    Dictionary dictionary;
    dictionary.solutions = readWords(kSolutionFile);
    std::vector<std::string> allWords = readWords(kAllWordsFile);

    for (const std::string &word : allWords) {
        dictionary.allowed.insert(word);
    }
    for (const std::string &word : dictionary.solutions) {
        dictionary.allowed.insert(word);
    }

    if (dictionary.solutions.empty() || dictionary.allowed.empty()) {
        throw std::runtime_error("Cannot load word lists. Keep ALL.TXT and SOLUTION.TXT next to the program.");
    }
    return dictionary;
}

bool isAllLetters(const std::string &word) {
    return word.size() == kWordLength &&
           std::all_of(word.begin(), word.end(), [](unsigned char ch) { return std::isalpha(ch); });
}

GuessResult scoreGuess(const std::string &guess, const std::string &answer) {
    GuessResult result;
    std::array<int, 26> remaining{};

    for (int i = 0; i < kWordLength; ++i) {
        if (guess[i] == answer[i]) {
            result.marks[i] = 2;
        } else {
            remaining[answer[i] - 'a']++;
        }
    }

    for (int i = 0; i < kWordLength; ++i) {
        if (result.marks[i] == 2) {
            continue;
        }
        int idx = guess[i] - 'a';
        if (idx >= 0 && idx < 26 && remaining[idx] > 0) {
            result.marks[i] = 1;
            remaining[idx]--;
        }
    }
    return result;
}

std::string coloredTile(char ch, int mark) {
    const char *color = mark == 2 ? kGreen : (mark == 1 ? kYellow : kGray);
    std::string tile = color;
    tile += ' ';
    tile += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    tile += ' ';
    tile += kReset;
    return tile;
}

void printBoard(const std::vector<std::string> &guesses, const std::vector<GuessResult> &results) {
    std::cout << "\nBoard\n";
    for (std::size_t row = 0; row < guesses.size(); ++row) {
        std::cout << "  ";
        for (int col = 0; col < kWordLength; ++col) {
            std::cout << coloredTile(guesses[row][col], results[row].marks[col]);
        }
        std::cout << '\n';
    }
    for (std::size_t row = guesses.size(); row < kMaxTries; ++row) {
        std::cout << "  ";
        for (int col = 0; col < kWordLength; ++col) {
            std::cout << kGray << "   " << kReset;
        }
        std::cout << '\n';
    }
}

void printKeyboard(const std::unordered_map<char, int> &keyboard) {
    const std::vector<std::string> rows = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
    std::cout << "\nKeyboard\n";
    for (const std::string &row : rows) {
        std::cout << "  ";
        for (char ch : row) {
            auto found = keyboard.find(ch);
            if (found == keyboard.end()) {
                std::cout << ' ' << static_cast<char>(std::toupper(ch)) << ' ';
            } else {
                std::cout << coloredTile(ch, found->second);
            }
        }
        std::cout << '\n';
    }
}

void updateKeyboard(std::unordered_map<char, int> &keyboard, const std::string &guess, const GuessResult &result) {
    for (int i = 0; i < kWordLength; ++i) {
        char ch = guess[i];
        auto existing = keyboard.find(ch);
        if (existing == keyboard.end() || result.marks[i] > existing->second) {
            keyboard[ch] = result.marks[i];
        }
    }
}

std::string emojiShare(const GameResult &game, bool dailyMode) {
    std::ostringstream out;
    out << "Wordle in C++ " << (dailyMode ? "Daily " : "")
        << (game.won ? std::to_string(game.attempts) : "X") << "/" << kMaxTries << "\n";
    for (const GuessResult &result : game.results) {
        for (int mark : result.marks) {
            out << (mark == 2 ? "🟩" : (mark == 1 ? "🟨" : "⬛"));
        }
        out << '\n';
    }
    return out.str();
}

Stats loadStats() {
    Stats stats;
    std::ifstream input(kStatsFile);
    if (!input) {
        return stats;
    }
    input >> stats.played >> stats.wins >> stats.currentStreak >> stats.bestStreak;
    for (int i = 1; i <= kMaxTries; ++i) {
        input >> stats.distribution[i];
    }
    return stats;
}

void saveStats(const Stats &stats) {
    std::ofstream output(kStatsFile);
    output << stats.played << ' ' << stats.wins << ' ' << stats.currentStreak << ' ' << stats.bestStreak << '\n';
    for (int i = 1; i <= kMaxTries; ++i) {
        output << stats.distribution[i] << (i == kMaxTries ? '\n' : ' ');
    }
}

void updateStats(const GameResult &game) {
    if (game.quietAnswer) {
        return;
    }
    Stats stats = loadStats();
    stats.played++;
    if (game.won) {
        stats.wins++;
        stats.currentStreak++;
        stats.bestStreak = std::max(stats.bestStreak, stats.currentStreak);
        stats.distribution[game.attempts]++;
    } else {
        stats.currentStreak = 0;
    }
    saveStats(stats);
}

void printStats() {
    Stats stats = loadStats();
    double winRate = stats.played == 0 ? 0.0 : (100.0 * stats.wins / stats.played);
    std::cout << "\nStats\n"
              << "  Played: " << stats.played << '\n'
              << "  Wins: " << stats.wins << " (" << std::fixed << std::setprecision(1) << winRate << "%)\n"
              << "  Current streak: " << stats.currentStreak << '\n'
              << "  Best streak: " << stats.bestStreak << "\n\nDistribution\n";
    int maxBucket = *std::max_element(std::begin(stats.distribution) + 1, std::end(stats.distribution));
    for (int i = 1; i <= kMaxTries; ++i) {
        int bars = maxBucket == 0 ? 0 : (30 * stats.distribution[i] / maxBucket);
        std::cout << "  " << i << ": " << std::string(bars, '#') << " " << stats.distribution[i] << '\n';
    }
}

std::string readLine(const std::string &prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        return "exit";
    }

    auto first = std::find_if_not(line.begin(), line.end(), [](unsigned char ch) { return std::isspace(ch); });
    auto last = std::find_if_not(line.rbegin(), line.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
    if (first >= last) {
        return "";
    }

    std::string out(first, last);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return out;
}

std::string readSecretLine(const std::string &prompt) {
    std::cout << prompt;
#ifdef _WIN32
    std::string word;
    for (;;) {
        int ch = _getch();
        if (ch == '\r' || ch == '\n') {
            std::cout << '\n';
            break;
        }
        if (ch == '\b') {
            if (!word.empty()) {
                word.pop_back();
                std::cout << "\b \b";
            }
        } else if (std::isprint(ch)) {
            word.push_back(static_cast<char>(ch));
            std::cout << '*';
        }
    }
    return trimAndLower(word);
#else
    termios oldAttr{};
    termios newAttr{};
    tcgetattr(STDIN_FILENO, &oldAttr);
    newAttr = oldAttr;
    newAttr.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newAttr);
    std::string word;
    std::getline(std::cin, word);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
    std::cout << '\n';
    return trimAndLower(word);
#endif
}

bool satisfiesHardMode(const std::string &guess,
                       const std::vector<std::string> &previousGuesses,
                       const std::vector<GuessResult> &previousResults,
                       std::string &message) {
    std::array<char, kWordLength> fixed{};
    std::array<int, 26> required{};

    for (std::size_t row = 0; row < previousGuesses.size(); ++row) {
        std::array<int, 26> rowRequired{};
        for (int col = 0; col < kWordLength; ++col) {
            int mark = previousResults[row].marks[col];
            char ch = previousGuesses[row][col];
            if (mark == 2) {
                fixed[col] = ch;
                rowRequired[ch - 'a']++;
            } else if (mark == 1) {
                rowRequired[ch - 'a']++;
            }
        }
        for (int i = 0; i < 26; ++i) {
            required[i] = std::max(required[i], rowRequired[i]);
        }
    }

    for (int col = 0; col < kWordLength; ++col) {
        if (fixed[col] != '\0' && guess[col] != fixed[col]) {
            message = "Hard mode: position " + std::to_string(col + 1) + " must be '" + fixed[col] + "'.";
            return false;
        }
    }

    std::array<int, 26> counts{};
    for (char ch : guess) {
        counts[ch - 'a']++;
    }
    for (int i = 0; i < 26; ++i) {
        if (counts[i] < required[i]) {
            message = "Hard mode: your guess must reuse the revealed letter '" + std::string(1, char('a' + i)) + "'.";
            return false;
        }
    }
    return true;
}

std::string chooseRandomAnswer(const std::vector<std::string> &solutions) {
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<std::size_t> dist(0, solutions.size() - 1);
    return solutions[dist(rng)];
}

std::string chooseDailyAnswer(const std::vector<std::string> &solutions) {
    using days = std::chrono::duration<long long, std::ratio<86400>>;
    auto now = std::chrono::system_clock::now().time_since_epoch();
    long long day = std::chrono::duration_cast<days>(now).count();
    return solutions[static_cast<std::size_t>(day % static_cast<long long>(solutions.size()))];
}

GameResult playGame(const Dictionary &dictionary, const std::string &answer, GameOptions options) {
    GameResult game;
    game.answer = answer;
    game.quietAnswer = options.quietAnswer;
    std::unordered_map<char, int> keyboard;
    auto start = std::chrono::steady_clock::now();

    std::cout << "\nGuess a " << kWordLength << "-letter word in " << kMaxTries << " tries."
              << (options.hardMode ? " Hard mode is ON." : "") << "\n";

    while (game.guesses.size() < kMaxTries) {
        printBoard(game.guesses, game.results);
        printKeyboard(keyboard);
        std::string guess = readLine("\nGuess " + std::to_string(game.guesses.size() + 1) + ": ");

        if (guess == "quit") {
            break;
        }
        if (!isAllLetters(guess)) {
            std::cout << kRed << " Please enter exactly five letters. " << kReset << "\n";
            continue;
        }
        if (!dictionary.allowed.count(guess)) {
            std::cout << kRed << " That word is not in the dictionary. " << kReset << "\n";
            continue;
        }
        std::string hardModeMessage;
        if (options.hardMode && !satisfiesHardMode(guess, game.guesses, game.results, hardModeMessage)) {
            std::cout << kRed << ' ' << hardModeMessage << ' ' << kReset << "\n";
            continue;
        }

        GuessResult result = scoreGuess(guess, answer);
        game.guesses.push_back(guess);
        game.results.push_back(result);
        updateKeyboard(keyboard, guess, result);
        game.attempts = static_cast<int>(game.guesses.size());

        if (guess == answer) {
            game.won = true;
            break;
        }
    }

    printBoard(game.guesses, game.results);
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count();
    if (game.won) {
        std::cout << "\nYou win in " << game.attempts << " tries! Time: " << elapsed << "s\n";
    } else {
        std::cout << "\nYou lose. The answer was " << answer << ". Time: " << elapsed << "s\n";
    }
    std::cout << "\nShare card:\n" << emojiShare(game, options.dailyMode);
    updateStats(game);
    return game;
}

void printHelp() {
    std::cout << R"(
Wordle in C++ Plus
------------------
- Green means the letter is correct and in the correct position.
- Yellow means the answer contains that letter, but somewhere else.
- Gray means the answer does not contain that letter in this amount.
- Hard mode forces you to reuse all revealed hints.
- Daily mode picks a stable answer for the current UTC day.
- Type 'quit' during a round to give up.
)";
}

void duel(const Dictionary &dictionary) {
    std::cout << "\nDuel mode: each player secretly enters a valid answer for the other player.\n";
    GameResult results[2];
    for (int round = 0; round < 2; ++round) {
        int setter = round + 1;
        int guesser = 2 - round;
        std::string answer;
        do {
            answer = readSecretLine("Player " + std::to_string(setter) + ", enter a secret answer for Player " + std::to_string(guesser) + ": ");
            if (!isAllLetters(answer) || !dictionary.allowed.count(answer)) {
                std::cout << "Invalid secret answer. It must be a valid five-letter dictionary word.\n";
            }
        } while (!isAllLetters(answer) || !dictionary.allowed.count(answer));

        GameOptions options;
        options.quietAnswer = true;
        results[round] = playGame(dictionary, answer, options);
    }

    std::cout << "\nDuel result\n";
    if (results[0].won != results[1].won) {
        std::cout << "Player " << (results[0].won ? 2 : 1) << " wins!\n";
    } else if (!results[0].won) {
        std::cout << "Both players missed. It is a tie.\n";
    } else if (results[0].attempts != results[1].attempts) {
        std::cout << "Player " << (results[0].attempts < results[1].attempts ? 2 : 1)
                  << " wins with fewer attempts!\n";
    } else {
        std::cout << "Both players solved in " << results[0].attempts << " attempts. It is a tie.\n";
    }
}

void menu(const Dictionary &dictionary) {
    for (;;) {
        std::cout << R"(
==============================
      Wordle in C++ Plus
==============================
1. Random single-player game
2. Daily challenge
3. Duel mode
4. Hard-mode random game
5. Stats
6. Help
0. Exit
)";
        std::string choice = readLine("Choose: ");
        if (choice == "0" || choice == "exit") {
            return;
        }
        if (choice == "1") {
            playGame(dictionary, chooseRandomAnswer(dictionary.solutions), GameOptions{});
        } else if (choice == "2") {
            GameOptions options;
            options.dailyMode = true;
            playGame(dictionary, chooseDailyAnswer(dictionary.solutions), options);
        } else if (choice == "3") {
            duel(dictionary);
        } else if (choice == "4") {
            GameOptions options;
            options.hardMode = true;
            playGame(dictionary, chooseRandomAnswer(dictionary.solutions), options);
        } else if (choice == "5") {
            printStats();
        } else if (choice == "6" || choice == "help") {
            printHelp();
        } else {
            std::cout << "Invalid choice.\n";
        }
    }
}

} // namespace

int main() {
    try {
        Dictionary dictionary = loadDictionary();
        menu(dictionary);
        return EXIT_SUCCESS;
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
