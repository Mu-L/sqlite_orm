#include <sqlite_orm/sqlite_orm.h>
#include <catch2/catch.hpp>
#include <type_traits>  //  std::is_same

using namespace sqlite_orm;

template<class St, class E, class V>
void runTest(V value) {
    using Type = internal::column_result_of_t<St, V>;
    static_assert(std::is_same<Type, E>::value, "");
}

TEST_CASE("column_result_of_t 1") {
    struct User {
        int id = 0;
        std::string name;
    };

    struct Visit {
        void setId(int value) {
            this->id = value;
        }

        int getId() const {
            return this->id;
        }

        void setComment(std::string comment) {
            this->comment = move(comment);
        }

        const std::string &getComment() const {
            return this->comment;
        }

      private:
        int id = 0;
        std::string comment;
    };
    auto storage = make_storage({});

    using Storage = decltype(storage);
    runTest<Storage, int>(&User::id);
    runTest<Storage, std::string>(&User::name);
    runTest<Storage, bool>(in(&User::id, {1, 2, 3}));
    {
        std::vector<int> vector;
        vector.push_back(1);
        vector.push_back(2);
        vector.push_back(3);
        runTest<Storage, bool>(in(&User::id, vector));
    }
    runTest<Storage, bool>(in(&User::id, select(&User::id)));
    runTest<Storage, bool>(c(&User::id).in(1, 2, 3));
    runTest<Storage, int>(&Visit::getId);
    runTest<Storage, std::string>(&Visit::getComment);
    runTest<Storage, int>(&Visit::setId);
    runTest<Storage, std::string>(&Visit::setComment);
    runTest<Storage, std::unique_ptr<double>>(sqlite_orm::abs(&User::id));
    runTest<Storage, int>(sqlite_orm::length(&User::id));
    runTest<Storage, int>(sqlite_orm::unicode(&User::id));
    runTest<Storage, std::string>(sqlite_orm::typeof_(&User::id));
    runTest<Storage, std::string>(sqlite_orm::lower(&User::id));
    runTest<Storage, std::string>(sqlite_orm::upper(&User::id));
    runTest<Storage, std::unique_ptr<int>>(max(&User::id));
    runTest<Storage, std::unique_ptr<std::string>>(max(&User::name));
    runTest<Storage, int>(count<User>());
    runTest<Storage, int>(count());
    {
        struct RandomFunc {
            int operator()() const {
                return 4;
            }
        };
        runTest<Storage, int>(func<RandomFunc>());
    }
    runTest<Storage, int>(distinct(&User::id));
    runTest<Storage, std::string>(distinct(&User::name));
    runTest<Storage, int>(all(&User::id));
    runTest<Storage, std::string>(all(&User::name));
    runTest<Storage, std::string>(conc(&User::name, &User::id));
    runTest<Storage, std::string>(c(&User::name) || &User::id);
    runTest<Storage, double>(add(&User::id, 5));
    runTest<Storage, double>(c(&User::id) + 5);
    runTest<Storage, double>(sub(&User::id, 5));
    runTest<Storage, double>(c(&User::id) - 5);
    runTest<Storage, double>(mul(&User::id, 5));
    runTest<Storage, double>(c(&User::id) * 5);
    runTest<Storage, double>(sqlite_orm::div(&User::id, 5));
    runTest<Storage, double>(c(&User::id) / 5);
    runTest<Storage, double>(mod(&User::id, 5));
    runTest<Storage, double>(c(&User::id) % 5);
    runTest<Storage, int>(bitwise_shift_left(&User::id, 4));
    runTest<Storage, int>(bitwise_shift_right(&User::id, 4));
    runTest<Storage, int>(bitwise_and(&User::id, 4));
    runTest<Storage, int>(bitwise_or(&User::id, 4));
    runTest<Storage, int>(bitwise_not(&User::id));
    runTest<Storage, int64>(rowid());
    runTest<Storage, int64>(oid());
    runTest<Storage, int64>(_rowid_());
    runTest<Storage, int64>(rowid<User>());
    runTest<Storage, int64>(oid<User>());
    runTest<Storage, int64>(_rowid_<User>());
}

TEST_CASE("column_result_of_t 2") {
    struct Org {
        // compile-time mapped (via c_v<>)
        int64 id = 0;
        // not compile-time mapped
        int64 boss = 0;
    };
    struct Derived : Org {};

    auto storage = make_storage(
        "",
        make_table("org", make_column("id", c_v<&Org::id>), make_column("boss", &Org::boss)),
        make_table<Derived>("derived", make_column("id", c_v<&Derived::id>), make_column("boss", &Derived::boss)));
    using storage_type = decltype(storage);

    runTest<storage_type, int64>(c_v<&Org::id>);
    runTest<storage_type, std::tuple<int64, int64>>(columns(c_v<&Org::id>, &Org::boss));
    runTest<storage_type, std::tuple<int64, int64>>(asterisk<Org>());
    runTest<storage_type, std::tuple<int64, int64>>(asterisk<alias_a<Org>>());
    runTest<storage_type, Org>(object<Org>());
    runTest<storage_type, int64>(column<Derived>(&Org::id));
    runTest<storage_type, int64>(column<cte_1>(c_v<&Org::id>));
    // this needs a 'CTE' expression storage
    //runTest<storage_type, int64>(column<cte_1>(0_col));
}
