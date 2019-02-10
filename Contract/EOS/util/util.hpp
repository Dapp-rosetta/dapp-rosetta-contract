#pragma once
#include <eosiolib/eosio.hpp>
#include <string>
#include <vector>

namespace kyubeyutil {

    // using namespace std;
    using std::string;
    using std::vector;
    using namespace eosio;

    struct st_transfer { // 190211 yukiexe
        name from;
        name to;
        asset        quantity;
        string       memo;

        EOSLIB_SERIALIZE( st_transfer, (from)(to)(quantity)(memo) )
    };

    const vector<string> explode(const string& s, const char& c) { // 190211 yukiexe
        string buff{""};
        vector<string> v;

        for(auto n:s) {
            if(n != c) buff+=n; 
            else if(n == c && buff != "") { v.push_back(buff); buff = ""; }
        }
        if(!buff.empty()) v.push_back(buff);

        return v;
    }

    const vector<string> split(const string& s, const char& t) {
        string buff;
        vector<string> z;

        for (auto c: s) {
            if (c != t) buff += c; 
            else { 
                z.push_back(buff); 
                buff.clear(); 
            }
        }
        if (!buff.empty()) z.push_back(buff);
        return z;
    }

    int64_t string_to_price(string s) { // 190211 yukiexe
        int64_t z = 0;
        for (int i=0;i<s.size();++i) {
            if ('0' <= s[i] && s[i] <= '9') {
                z *= 10; 
                z += s[i] - '0';
            }
        }
        return z;
    }

    uint64_t string_to_int(string s) {  // 190211 yukiexe
        uint64_t z = 0;
        for (int i=0;i<s.size();++i) {
            if ('0' <= s[i] && s[i] <= '9') {
                z *= 10; 
                z += s[i] - '0';
            }
        }
        return z;
    }

    string int_to_string(uint64_t t) {
        if (t == 0) return "0";
        string z;
        while (t != 0) {
            z += char('0' + (t % 10));  
            t /= 10;
        }
        reverse(z.begin(), z.end());
        return z;
    }

    /**
     * @dev minakokojima
     * bet 50 ludufutemp minakokojima
     */
    class stringSplitter { 
    public:
      stringSplitter(const string& _str) : str(_str) {
          current_position = 0;
      }

      bool eof() { return current_position == str.length(); }

      void skip_empty() {
          while (!eof() && str[current_position] == ' ') current_position ++;
      }

      bool get_char(char* ch) {
          if (!eof()) {
              *ch  = str[current_position++];
              if (*ch == ' ') return false;
              else return true;
          } else return false;
      }

      void get_string(string* result) {
          result->clear();
          skip_empty();
          // if (eof()) return -1;
          //eosio_assert(!eof(), "No enough chars.");
          char ch;
          while (get_char(&ch)) {
              *result+= ch;
              //current_position++;
          }
          skip_empty();
      }

      void get_uint(uint64_t* result) {
          skip_empty();
          *result = 0;
          char ch;
          while (get_char(&ch)) {
              eosio_assert(ch >= '0' && ch <= '9', "Should be a valid number");
              *result = *result * 10 + ch - '0';
          }
          skip_empty();
      }
      
    private:
      string str;
      int current_position;
    };

} /// namespace kyubeyutil