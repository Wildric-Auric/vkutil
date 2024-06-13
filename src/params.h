#pragma once

enum class MSAAvalue {
    x1  = 1,
    x2  = 2,
    x4  = 4,
    x8  = 8,
    x16 = 16,
    x32 = 32,
    x64 = 64
};
     
     
class GfxParams {   
    public: 
        bool      anisotropy = 1;
        MSAAvalue msaa       = MSAAvalue::x1;
        static GfxParams inst;
    private:
        GfxParams() = default;

};
