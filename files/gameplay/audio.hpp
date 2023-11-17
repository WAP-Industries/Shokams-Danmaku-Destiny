std::map<std::string, std::pair<Mix_Chunk*, int>> AudioLib;

void LoadAudio(std::string path, int channel){
    Mix_Chunk* Audio= Mix_LoadWAV(path.c_str());
    if (Audio== NULL)
        throw std::runtime_error("Failed to load audio: "+path);
    AudioLib[path] = {Audio, channel};
}

void PlayAudio(std::string path){
    if (
        Game.Ending.has_value() && 
        path.find("ending\\")==std::string::npos
    ) return;
    Mix_PlayChannel(AudioLib[path].second, AudioLib[path].first, 0);
}

void PlayMusic(std::string name){
    std::string src = "assets\\audio\\music\\"+name+".wav";
    if (Game.CurrentMusic==src) return;
    Game.CurrentMusic = src;
    Mix_PlayMusic(Music[src.c_str()], -1);
}