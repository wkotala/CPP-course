#ifndef FUNCLIST_H
#define FUNCLIST_H

#include <functional>
#include <ranges>
#include <sstream>
#include <string>

namespace flist {
    // Empty list.
    inline auto empty = []<typename F, typename A>(F, A a) -> A {
        return a;
    };

    // Returns a new list by prepending list `l` with element `x`.
    inline auto cons = [](auto x, auto l) {
        return [x, l]<typename F, typename A>(F f, A a) -> A {
            return f(x, l(f, a));
        };
    };

    namespace detail {
        inline auto create() {
            return empty;
        }

        inline auto create(auto x, auto... xs) {
            return cons(x, create(xs...));
        }
    }

    // Returns a list created from the given elements.
    inline auto create = [](auto... xs) {
        return detail::create(xs...);
    };

    // Begin and end iterators for ranges possibly wrapped in std::reference_wrapper.
    namespace detail {
        template <std::ranges::bidirectional_range R>
        auto range_begin(R& range) {
            return std::ranges::begin(range);
        }

        template <std::ranges::bidirectional_range R>
        auto range_begin(std::reference_wrapper<R> range) {
            return std::ranges::begin(range.get());
        }

        template <std::ranges::bidirectional_range R>
        auto range_end(R& range) {
            return std::ranges::end(range);
        }

        template <std::ranges::bidirectional_range R>
        auto range_end(std::reference_wrapper<R> range) {
            return std::ranges::end(range.get());
        }
    }

    // Returns a list created from the elements of the given range.
    inline auto of_range = [](auto range) {
        return [range]<typename F, typename A>(F f, A a) -> A {
            auto begin = detail::range_begin(range);
            auto end = detail::range_end(range);
            using Iterator = decltype(begin);

            std::function<A(Iterator)> recursive_of_range = [&](Iterator it) {
                if (it == end) {
                    return a;
                } else {
                    auto x{*it};
                    return f(x, recursive_of_range(++it));
                }
            };

            return recursive_of_range(begin);
        };
    };

    // Returns concatenation of the given lists.
    inline auto concat = [](auto l, auto k) {
        return [l, k]<typename F, typename A>(F f, A a) -> A {
            return l(f, k(f, a));
        };
    };

    // Returns reverse of the given list.
    inline auto rev = [](auto l) {
        return [l]<typename F, typename A>(F f, A a) -> A {
            std::function<A(A)> alpha_0 = [](A a) { return a; }; // identity function
            
            auto phi = [&f](auto x, std::function<A(A)> alpha) -> std::function<A(A)> {
                return [&f, x, alpha](A a) {
                    return alpha(f(x, a));
                };
            };

            return l(phi, alpha_0)(a);
        };
    };

    // Returns a list created by applying function `m` to each element of list `l`.
    inline auto map = [](auto m, auto l) {
        return [m, l]<typename F, typename A>(F f, A a) -> A {
            return l([&m, &f](auto x, A a) {
                return f(m(x), a);
            }, a);
        };
    };

    // Returns a list created by filtering elements of list `l` with predicate `p`.
    inline auto filter = [](auto p, auto l) {
        return [p, l]<typename F, typename A>(F f, A a) -> A {
            return l([&p, &f](auto x, A a) {
                return p(x) ? f(x, a) : a;
            }, a);
        };
    };

    
    // Returns a flattened list, created by concatenating the nested lists in list `ll`.
    inline auto flatten = [](auto ll) {
        return [ll]<typename F, typename A>(F f, A a) -> A {
            return ll([&f](auto l, A a) {
                return l(f, a);
            }, a);
        };
    };

    // Returns string representation of the given list, assuming that the elements are printable with `operator<<`.
    inline auto as_string = [](const auto& l) -> std::string {
        std::vector<std::string> elements;

        l([&elements](auto x, auto) {
            std::ostringstream oss;
            oss << x << ';';
            elements.push_back(oss.str());
            return 0;
        }, 0);

        std::string result = "[";
        for (auto it = elements.crbegin(); it != elements.crend(); ++it) {
            result += *it;
        }

        if (result.size() > 1) {
            result.pop_back();
        }
        result += "]";
        
        return result;
    };

} // namespace flist

#endif // FUNCLIST_H
