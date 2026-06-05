#pragma once
#include <cmath>
#include <limits>
#include <algorithm>
struct EaseData
{
    float timer;
    float totalTime = 1.0f;
    float endValue;//maxValue
    float startValue;//minValue
    float backValue = 1.70158f;//back
};

// イージング関数
struct Easing
{
private:
    template <class Ty = double>
    static constexpr Ty Pai{ static_cast<Ty>(3.141592653589793) }; // 円周率

    template<typename Ty>
    static constexpr Ty _0 = static_cast<Ty>(0);
    template<typename Ty>
    static constexpr Ty _0_5 = static_cast<Ty>(0.5);
    template<typename Ty>
    static constexpr Ty _0_75 = static_cast<Ty>(0.75);
    template<typename Ty>
    static constexpr Ty _0_9375 = static_cast<Ty>(0.9375);
    template<typename Ty>
    static constexpr Ty _0_984375 = static_cast<Ty>(0.984375);

    template<typename Ty>
    static constexpr Ty _1 = static_cast<Ty>(1);
    template<typename Ty>
    static constexpr Ty _1_5 = static_cast<Ty>(1.5);
    template<typename Ty>
    static constexpr Ty _1_525 = static_cast<Ty>(1.525);

    template<typename Ty>
    static constexpr Ty _2 = static_cast<Ty>(2);
    template<typename Ty>
    static constexpr Ty _2_25 = static_cast<Ty>(2.25);
    template<typename Ty>
    static constexpr Ty _2_5 = static_cast<Ty>(2.5);
    template<typename Ty>
    static constexpr Ty _2_625 = static_cast<Ty>(2.625);
    template<typename Ty>
    static constexpr Ty _2_75 = static_cast<Ty>(2.75);

    template<typename Ty>
    static constexpr Ty _7_5625 = static_cast<Ty>(7.5625);

    template<typename Ty>
    static constexpr Ty _10 = static_cast<Ty>(10);

    template<typename Ty>
    static constexpr Ty _180 = static_cast<Ty>(180);

    template<typename Ty>
    static constexpr Ty _90 = static_cast<Ty>(90);

private:
    template<class Ty>
    [[nodiscard]] static inline Ty ToRadian(const Ty angle)
    {
        return static_cast<Ty>(angle * Pai<Ty> / _180<Ty>);
    }

    template<typename Ty>
    [[nodiscard]] static inline bool AdjEqual(const Ty epsilon_num, const Ty num)
    {
        auto Fabs{ [](const Ty num) constexpr {
            if (num > _0<Ty>) return num; else return -num;
        } };

        static constexpr auto Epsilon{ std::numeric_limits<Ty>::epsilon() };
        auto dis{ Fabs(epsilon_num - num) };

        return (dis <= Epsilon);
    }

public:

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InQuad(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        return max * time * time + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty OutQuad(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        return -max * time * (time - _2<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InOutQuad(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        if (time / _2<Ty> < _1<Ty>)
            return max / _2<Ty> *time * time + min;

        --time;

        return -max * (time * (time - _2<Ty>) - _1<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InCubic(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        return max * time * time * time + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty OutCubic(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time = time / totaltime - _1<Ty>;

        return max * (time * time * time + _1<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InOutCubic(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        if (time / _2<Ty> < _1<Ty>)
            return max / _2<Ty> *time * time * time + min;

        time -= _2<Ty>;

        return max / _2<Ty> *(time * time * time + _2<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InQuart(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        return max * time * time * time * time + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty OutQuart(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time = time / totaltime - _1<Ty>;

        return -max * (time * time * time * time - _1<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InOutQuart(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        if (time / _2<Ty> < _1<Ty>)
            return max / _2<Ty> *time * time * time * time + min;

        time -= _2<Ty>;

        return -max / _2<Ty> *(time * time * time * time - _2<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InQuint(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        return max * time * time * time * time * time + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty OutQuint(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time = time / totaltime - _1<Ty>;

        return max * (time * time * time * time * time + _1<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InOutQuint(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        if (time / _2<Ty> < _1<Ty>)
            return max / _2<Ty> *time * time * time * time * time + min;

        time -= _2<Ty>;

        return max / _2<Ty> *(time * time * time * time * time + _2<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InSine(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;

        return -max * std::cos(time * ToRadian(_90<Ty>) / totaltime) + max + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty OutSine(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;

        return max * std::sin(time * ToRadian(_90<Ty>) / totaltime) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InOutSine(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;

        return -max / _2<Ty> *(std::cos(time * Pai<Ty> / totaltime) - 1) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InExp(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;

        return  AdjEqual<Ty>(time, _0<Ty>) ?
            min :
            max * std::pow(_2<Ty>, _10<Ty> *(time / totaltime - _1<Ty>)) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty OutExp(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;

        return AdjEqual<Ty>(time, totaltime) ?
            max + min :
            max * (-std::pow(_2<Ty>, -_10<Ty> *time / totaltime) + _1<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InOutExp(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        if (AdjEqual<Ty>(time, _0<Ty>))
            return min;

        if (AdjEqual<Ty>(time, totaltime))
            return max;

        max -= min;
        time /= totaltime;

        if (time / _2<Ty> < _1<Ty>)
            return max / _2<Ty> *std::pow(_2<Ty>, _10<Ty> *(time - _1<Ty>)) + min;

        --time;

        return max / _2<Ty> *(-std::pow(_2<Ty>, -_10<Ty> *time) + _2<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InCirc(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;
        time = (std::max)((std::min)(time, _1<Ty>), -_1<Ty>);


        return -max * (std::sqrt(_1<Ty> -time * time) - _1<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty OutCirc(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= (totaltime - _1<Ty>);
        time = (std::max)((std::min)(time, _1<Ty>), -_1<Ty>);


        return max * std::sqrt(_1<Ty> -time * time) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InOutCirc(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;
        time = (std::max)((std::min)(time, _1<Ty>), -_1<Ty>);


        if (time / _2<Ty> < _1<Ty>)
            return -max / _2<Ty> *(std::sqrt(_1<Ty> -time * time) - _1<Ty>) + min;

        time -= _2<Ty>;

        return max / _2<Ty> *(std::sqrt(_1<Ty> -time * time) + _1<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InBack(Ty time, Ty totaltime, Ty back, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        return max * time * time * ((back + _1<Ty>) * time - back) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty OutBack(Ty time, Ty totaltime, Ty back, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time = time / totaltime - _1<Ty>;
		return max * (time * time * ((back + _1<Ty>) * time + back) + _1<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InOutBack(Ty time, Ty totaltime, Ty back, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;
        back *= _1_525<Ty>;
        if (time / _2<Ty> < _1<Ty>)
            return max / _2<Ty> * (time * time * ((back + _1<Ty>) * time - back)) + min;
        time -= _2<Ty>;
		return max / _2<Ty> *(time * time * ((back + _1<Ty>) * time + back) + _2<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty OutBounce(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        max -= min;
        time /= totaltime;

        if (time < _1<Ty> / _2_75<Ty>)
            return max * (_7_5625<Ty> *time * time) + min;

        else if (time < _2<Ty> / _2_75<Ty>)
        {
            time -= _1_5<Ty> / _2_75<Ty>;

            return max * (_7_5625<Ty> *time * time + _0_75<Ty>) + min;
        }
        else if (time < _2_5<Ty> / _2_75<Ty>)
        {
            time -= _2_25<Ty> / _2_75<Ty>;

            return max * (_7_5625<Ty> *time * time + _0_9375<Ty>) + min;
        }
        else
        {
            time -= _2_625<Ty> / _2_75<Ty>;

            return max * (_7_5625<Ty> *time * time + _0_984375<Ty>) + min;
        }
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InBounce(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        return max - OutBounce<Ty>(totaltime - time, totaltime, max - min, _0<Ty>) + min;
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty InOutBounce(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        if (time < totaltime / _2<Ty>)
        {
            return InBounce<Ty>(time * _2<Ty>, totaltime, max - min, max) * _0_5<Ty> +min;
        }
        else
        {
            return OutBounce<Ty>(time * _2<Ty> -totaltime, totaltime, max - min, _0<Ty>) * _0_5<Ty> +min + (max - min) * _0_5<Ty>;
        }
    }

    template<typename Ty = float>
    [[nodiscard]] static inline Ty Linear(Ty time, Ty totaltime, Ty max = _1<Ty>, Ty min = _0<Ty>)
    {
        return (max - min) * time / totaltime + min;
    }
};
