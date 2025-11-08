# CPP-course

This repository contains three chosen header-only libraries, which are my solutions to tasks from the C++ course at the University of Warsaw during the 2024/2025 academic year.

---

## 1. Functional Lists

*   **File:** `funclist.h`
*   **Description:** This library provides a set of tools for working with functional-style, immutable lists. Lists are not represented as traditional data structures but as callable objects that implicitly hold a sequence of elements. All operations (like `map`, `filter`, `concat`, `reverse`) are implemented as constant function objects, promoting a functional programming paradigm. The core of the implementation relies heavily on C++ lambda expressions.
*   **Key Learnings:** Functional programming in C++, advanced use of lambda expressions, `std::function`, and template metaprogramming.

---

## 2. Binder

*   **File:** `binder.h`
*   **Description:** A template class `binder<K, V>` that acts as an associative container. Its main feature is the implementation of the copy-on-write optimization technique. This means that copying the container is a cheap operation, and a deep copy of the underlying data is only performed when a modification is made to one of the copies. The implementation guarantees strong exception safety and uses smart pointers for automatic memory management. It also includes a custom `const_iterator` for traversing the elements.
*   **Key Learnings:** Copy-on-write semantics, strong exception safety guarantees, memory management with smart pointers, template class design, and implementation of custom iterators compliant with standard library concepts.

---

## 3. Polynomials

*   **File:** `poly.h`
*   **Description:** A header-only library for performing compile-time and runtime arithmetic on multivariate polynomials. The `poly<T, N>` template class represents a polynomial where coefficients of type `T` can themselves be polynomials, allowing for a recursive representation of multiple variables. The entire implementation is `constexpr`, enabling complex polynomial calculations to be performed during compilation.
*   **Key Learnings:** Advanced template metaprogramming, extensive use of `constexpr` for compile-time computation, variadic templates, perfect forwarding, type traits, C++20 concepts, operator overloading, and class template argument deduction with custom deduction guides.

---

### Collaboration

*   **Functional Lists:** Implemented individually.
*   **Binder & Polynomials:** Implemented in a group of two.
