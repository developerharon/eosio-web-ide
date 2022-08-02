#include <eosio/eosio.hpp>
#include "abcounter.cpp"

using namespace eosio;

class [[eosio::contract("addressbook")]] addressbook : public eosio::contract {
    public:
        addressbook(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds) {

        }

        [[eosio::action]]
        void upsert(name user, std::string first_name, std::string last_name, uint64_t age, std::string street, std::string city, std::string state) {
            require_auth(user);
            address_index addresses(get_self(), get_first_receiver().value);
            auto iterator = addresses.find(user.value);

            if (iterator == addresses.end()) {
                // The user isn't in the table 
                addresses.emplace(user, [&]( auto& row) {
                    row.key = user;
                    row.first_name = first_name;
                    row.last_name = last_name;
                    row.age = age;
                    row.street = street;
                    row.city = city;
                    row.state = state;
                });
                send_summary(user, " successfully emplaced record to addressbook");
                increment_counter(user, "emplace");
            }
            else {
                // The user is in the table
                std::string changes;

                addresses.modify(iterator, user, [&]( auto& row) {
                    

                    if (row.first_name != first_name) {
                        row.first_name = first_name;
                        changes += "first name ";
                    }

                    if (row.last_name != last_name) {
                        row.last_name = last_name;
                        changes += "last name ";
                    }

                    if (row.age != age) {
                        row.age = age;
                        changes += "age ";
                    }

                    if (row.street != street){
                        row.street = street;
                        changes += "street ";
                    }

                    if (row.city != city) {
                        row.city = city;
                        changes += "city ";
                    }

                    if (row.state != state) {
                        row.state = state;
                        changes != "state "
                    }
                });
                if (!changes.empty()) {
                    send_summary(user, " successfully modified reocrd in addressbok. Fields changed: " + changes);
                    increment_counter(user, "modify");
                }
                else {
                    send_summary(user, " called upsert, but request resulted in no changes.");
                }
            }
        }

        [[eosio::action]]
        void erase(name user) {
            require_auth(user);
            address_index addresses(get_self(), get_first_receiver().value);
            auto iterator = addresses.find(user.value);
            check(iterator != addresses.end(), "Record does not exist");
            addresses.erase(iterator);
            send_summary(user, " successfully erased record from addressbook");
            increment_counter(user, "erase");
        }

        [[eosio::action]]
        void notify(name user, std::string msg) {
            require_auth(get_self());
            require_recipient(user);
        }

    private:
        struct [[eosio::table]] person {
            name key;
            std::string first_name;
            std::string last_name;
            uint64_t age;
            std::string street;
            std::string city;
            std::string state;

            uint64_t primary_key() const { return key.value; }
            uint64_t get_secondary_1() const { return age; }
        };

        void send_summary(name user, std::string message) {
            action(
                // permission_level,
                permission_level{get_self(), "active"_n},
                // code,
                get_self(),
                // action,
                "notify"_n,
                // data
                std::make_tuple(user, name{user}.to_string() + message)

            ).send();
        }

        void increment_counter(name user, std::string type) {
            abcounter::count_action count("abcounter"_n, {get_self(), "active"_n});
            count.send(user, type);
        }
    
    using address_index = eosio::multi_index<"people"_n, person, indexed_by<"byage"_n, const_mem_fun<person, uint64_t, &person::get_secondary_1>>>;
};

