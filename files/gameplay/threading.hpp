struct Threading{
    static void RunAsync(std::function<void()> Callback){
        {
            std::thread T(Callback);
            T.detach();
        }
    }
    static void Delay(int Milliseconds){
        std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));
    }
};