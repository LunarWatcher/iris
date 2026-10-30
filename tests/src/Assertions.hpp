#pragma once

#define REQUIRE_EXCEPTION(expr, type) [&]() -> type {   \
        try {                                           \
            expr;                                       \
            FAIL("Expression did not throw");           \
        } catch (const type& t) {                       \
            return t;                                   \
        }                                               \
    }();

