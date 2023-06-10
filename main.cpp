#include <array>
#include <vector>
#include <tuple>
#include "print.h"

template <class T0, class ...Ts>
void printarray(T0 t0, Ts ...ts) {
    printnl(t0);
    ((printnl(", "), printnl(ts)), ...);
    printnl("\n");
}

template <class ...Ts>
auto mkarray(Ts ...ts) {
    using T = std::common_type_t<Ts...>;
    return std::array<T, sizeof...(Ts)>{ts...};
}

template <class T1, class T2>
struct common_type_two {
    using type = decltype(0 ? std::declval<T1>() : std::declval<T2>());
};

template <class ...Ts>
struct common_type {
};

template <class T0>
struct common_type<T0> {
    using type = T0;
};

template <class T0, class T1, class ...Ts>
struct common_type<T0, T1, Ts...> {
    using type = typename common_type_two<T0, typename common_type<T1, Ts...>::type>::type;
};

template <class T>
struct dummy {
    static consteval T declval() {
        return std::declval<T>();
    }
};

template <class T0, class ...Ts>
constexpr auto get_common_type(dummy<T0> t0, dummy<Ts> ...ts) {
    if constexpr (sizeof...(Ts) == 0) {
        return t0;
    } else {
        return dummy<decltype(0 ? t0.declval() : get_common_type(ts...).declval())>{};
    }
}

struct Animal {
};

struct Cat : Animal {
    Cat(Cat &&) = delete;
};

struct common_type_wrapper {
    template <class ...Ts>
    struct rebind {
        using type = typename common_type<Ts...>::type;
    };
};

struct tuple_wrapper {
    template <class ...Ts>
    struct rebind {
        using type = std::tuple<Ts...>;
    };
};

struct variant_wrapper {
    template <class ...Ts>
    struct rebind {
        using type = std::variant<Ts...>;
    };
};

template <class Tmpl, class Tup>
struct tuple_apply {
};

template <class Tmpl, class ...Ts>
struct tuple_apply<Tmpl, std::tuple<Ts...>> {
    using type = typename Tmpl::template rebind<Ts...>::type;
};

template <class Tup>
struct tuple_size {
};

template <>
struct tuple_size<std::tuple<>> {
    static constexpr size_t value = 0;
};

template <class T0, class ...Ts>
struct tuple_size<std::tuple<T0, Ts...>> {
    static constexpr size_t value = tuple_size<std::tuple<Ts...>>::value + 1;
};

template <size_t N>
struct array_wrapper {
    template <class T>
    struct rebind {
        using type = std::array<T, N>;
    };
};

struct vector_wrapper {
    template <class T>
    struct rebind {
        using type = std::vector<T>;
    };
};

template <class Tmpl, class Tup>
struct tuple_map {
};

template <class Tmpl, class ...Ts>
struct tuple_map<Tmpl, std::tuple<Ts...>> {
    using type = std::tuple<typename Tmpl::template rebind<Ts>::type...>;
};

template <class Tup1, class Tup2>
struct tuple_cat {
};

template <class ...T1s, class ...T2s>
struct tuple_cat<std::tuple<T1s...>, std::tuple<T2s...>> {
    using type = std::tuple<T1s..., T2s...>;
};

template <class T1, class Tup2>
struct tuple_push_front {
};

template <class T1, class ...T2s>
struct tuple_push_front<T1, std::tuple<T2s...>> {
    using type = std::tuple<T1, T2s...>;
};

template <class Tup>
struct tuple_front {
};

template <class T0, class ...Ts>
struct tuple_front<std::tuple<T0, Ts...>> {
    using type = T0;
};

template <size_t I, class Tup>
struct tuple_element {
};

template <class T0, class ...Ts>
struct tuple_element<0, std::tuple<T0, Ts...>> {
    using type = T0;
};

template <size_t N, class T0, class ...Ts>
struct tuple_element<N, std::tuple<T0, Ts...>> {
    using type = typename tuple_element<N - 1, std::tuple<Ts...>>::type;
};

/* template <class Tup> */
/* struct tuple_is_all_integral { */
/* }; */
/*  */
/* template <> */
/* struct tuple_is_all_integral<std::tuple<>> { */
/*     static constexpr bool value = true; */
/* }; */
/*  */
/* template <class T0, class ...Ts> */
/* struct tuple_is_all_integral<std::tuple<T0, Ts...>> { */
/*     static constexpr bool value = std::is_integral_v<T0> && tuple_is_all_integral<std::tuple<Ts...>>::value; */
/* }; */

template <class Tmpl, class Tup>
struct tuple_is_all {
};

template <class Tmpl, class ...Ts>
struct tuple_is_all<Tmpl, std::tuple<Ts...>> {
    static constexpr bool value = (true && ... && Tmpl::template rebind<Ts>::value);
};

template <class Tup>
struct tuple_variant_conversion {
};

template <class ...Ts>
struct tuple_variant_conversion<std::tuple<Ts...>> {
    using type = std::variant<Ts...>;
};

template <class ...Ts>
struct tuple_variant_conversion<std::variant<Ts...>> {
    using type = std::tuple<Ts...>;
};

template <size_t Beg, size_t End, class Lambda>
void static_for(Lambda lambda) {
    if constexpr (Beg < End) {
        std::integral_constant<size_t, Beg> i;
        struct Breaker {
            bool *m_broken;

            constexpr void static_break() const {
                *m_broken = true;
            }
        };
        if constexpr (std::is_invocable_v<Lambda, decltype(i), Breaker>) {
            bool broken = false;
            lambda(i, Breaker{&broken});
            if (broken) {
                return;
            }
        } else {
            lambda(i);
        }
        static_for<Beg + 1, End>(lambda);
    }
}

template <size_t ...Is, class Lambda>
void _static_for_impl(Lambda lambda, std::index_sequence<Is...>) {
    (lambda(std::integral_constant<size_t, Is>{}), ...);
    /* std::make_index_sequence<4>; */
    /* std::index_sequence<0, 1, 2, 3>; */
}

template <size_t N, class Lambda>
void static_for(Lambda lambda) {
    _static_for_impl(lambda, std::make_index_sequence<N>{});
}

/* template <class Lambda, class ...Ts> */
/* auto tup_map(Lambda lambda, std::tuple<Ts...> tup) { */
/*     std::tuple<std::invoke_result_t<Lambda, Ts>...> res; */
/*     static_for<sizeof...(Ts)>([&] (auto i) { */
/*         std::get<i.value>(res) = lambda(std::get<i.value>(tup)); */
/*     }); */
/*     return res; */
/* } */

template <class Lambda, class Tup, size_t ...Is>
auto _tup_map_impl(Lambda lambda, Tup tup, std::index_sequence<Is...>) {
    return std::tuple<std::invoke_result_t<Lambda, std::tuple_element_t<Is, Tup>>...>{
        lambda(std::get<Is>(tup))...
    };
}

template <class Lambda, class Tup>
auto tup_map(Lambda lambda, Tup tup) {
    return _tup_map_impl(lambda, tup, std::make_index_sequence<std::tuple_size_v<Tup>>{});
    /* {lambda(std::get<Is>(tup))...} */
    /* {lambda(get<0>(tup)), lambda(get<1>(tup)), ...} */
    /* lambda(std::get<Is>(tup)...) */
    /* lambda(get<0>(tup), get<1>(tup), ...) */
}

template <class Lambda, class Tup, size_t ...Is>
auto _tup_apply_impl(Lambda lambda, Tup tup, std::index_sequence<Is...>) {
    return lambda(std::get<Is>(tup)...);
}

template <class Lambda, class Tup>
auto tup_apply(Lambda lambda, Tup tup) {
    return _tup_apply_impl(lambda, tup, std::make_index_sequence<std::tuple_size_v<Tup>>{});
}

template <class T, class ...Ts>
struct is_same_any {
    static constexpr bool value = (false || ... || std::is_same_v<T, Ts>);
};

template <class T, class ...Ts>
struct is_convertible_any {
    static constexpr bool value = (false || ... || std::is_convertible_v<T, Ts>);
};

template <class T0, class ...Ts>
    requires ((true && ... && std::is_convertible_v<Ts, T0>))
std::array<T0, sizeof...(Ts) + 1> myvec(T0 t0, Ts ...ts) {
    return {t0, ts...};
}

int main() {
    print(is_convertible_any<std::string, bool>::value);
    std::tuple<int, float> tup = {42, 3.14f};
    auto restup = tup_map([] (auto x) { return x + 1; }, tup);
    print(restup);
    /* auto resval = tup_apply([] (int i, float f) { return i + f; }, tup); */
    auto resval = tup_apply([] (auto ...xs) { return (xs + ...); }, tup);
    print(resval);
    /* std::variant<int, float, double, char> var = 'c'; */
    /* static_for<0, 4>([&] (auto i) { */
    /*     if (i.value == var.index()) { */
    /*         print(std::get<i.value>(var)); */
    /*     } */
    /* }); */
    /* using Tup = std::tuple<int, float, double>; */
    /* using Tup2 = std::tuple<std::vector<double>, std::array<int, 4>>; */
    /* using Var = std::variant<int, float, double>; */
    /* constexpr auto i = tuple_size<Tup>::value; */
    /* using what = tuple_apply<common_type_wrapper, Var>::type; */
    /* using what2 = common_type<int, float, double>::type; */
    /* using what3 = tuple_apply<variant_wrapper, Tup>::type; */
    /* using what4 = tuple_map<array_wrapper<3>, Tup>::type; */
    /* using what5 = tuple_map<vector_wrapper, Tup>::type; */
    /* using what6 = tuple_cat<Tup, Tup2>::type; */
    /* using what7 = tuple_push_front<char *, Tup>::type; */
    /* using what8 = tuple_front<Tup>::type; */
    /* using what9 = tuple_element<1, Tup>::type; */
    /* constexpr auto i2 = tuple_is_all_integral<std::tuple<int, char, short>>::value; */
    /* using what = decltype(get_common_type(dummy<Cat>{}, dummy<Animal>{})); */
    /* print(mkarray(1, 2, 3)); */
    /* printarray(1, 2, 3); */
    return 0;
}
