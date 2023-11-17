struct {
    int StartSprites;
    
    std::optional<int> ScreenLimit = std::nullopt;

    std::optional<int> Ending = std::nullopt;

    bool Running = false;
    bool isEndScreen = false;
    bool isRewinding = false;
    bool isTransitioning = false;
    bool isPaused = false;

    std::optional<std::function<void()>> CurrentAttack = std::nullopt;
    std::optional<std::string> LastAttack = std::nullopt;
    
    int SelectState = 0;
    int SelectFontSize = 35;
    std::vector<std::string> SelectText = {"Play", "Controls", "Credits"};

    std::string CurrentMusic;
} Game;