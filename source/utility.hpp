#pragma once

struct NoCopy {
    NoCopy() = default;

    NoCopy(const NoCopy &) = delete;
    NoCopy &operator=(const NoCopy &) = delete;
};

struct NoMove {
    NoMove() = default;

    NoMove(NoMove &&) = delete;
    NoMove &operator=(NoMove &&) = delete;
};
