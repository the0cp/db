#include <iostream>
#include <map>
#include <iomanip>
#include <vector>
#include <string>
#include "colors.h"

class TextTable{
    public:
        typedef std::vector<std::string> Row;
        enum class Alignment{LEFT, RIGHT};

    private:
        char _horizontal;
        char _vertical;
        char _corner;
        Row _current;
        std::vector<Row> _rows;
        std::vector<unsigned>mutable _widths;
        std::map<unsigned, Alignment>mutable _alignment;

        static std::string repeat(unsigned times, char c){
            std::string result;
            for(;times>0; times--)  result += c;
            return result;
        }

        unsigned columns() const{
            return _rows[0].size();
        }

        void determineWidth() const{
            _widths.assign(columns(), 0);
            for(auto rowIterator = _rows.begin(); rowIterator != _rows.end(); rowIterator++){
                Row const &row = *rowIterator;
                
                for(unsigned i = 0; i < row.size(); i++){
                    _widths[i] = _widths[i] > row[i].size() ? _widths[i] : row[i].size();
                }
            }
        }

        void setupAlignment() const{
            for(unsigned i = 0; i < columns(); i++){
                if(_alignment.find(i) == _alignment.end()){
                    _alignment[i] = Alignment::LEFT;
                }
            }
        }

    public:
        TextTable(char horizontal = '-', char vertical = '|', char corner = '+') 
            : _horizontal(horizontal), 
              _vertical(vertical), 
              _corner(corner){}
        
        void setAlignment(unsigned i, Alignment alignment){
            _alignment[i] = alignment;
        }

        Alignment alignment(unsigned i) const{
            return _alignment[i];
        }

        char vertical() const{
            return _vertical;
        }

        char horizontal() const{
            return _horizontal;
        }

        std::vector<Row>const& rows() const{
            return _rows;
        }

        int width(unsigned i) const{
            return _widths[i];
        }

        void add(std::string const& content){
            _current.push_back(content);
        }

        void endRow(){
            _rows.push_back(_current);
            _current.assign(0, "");
        }

        template<typename Interator>
        void addRow(Interator begin, Interator end){
            for(auto i = begin; i != end; i++){
                add(*i);
            }
        }

        template<typename Container>
        void addRow(Container const& container){
            addRow(container.begin(), container.end());
        }

        void setup() const{
            determineWidth();
            setupAlignment();
        }

        std::string ruler() const{
            std::string result;
            result += _corner;
            for(auto cnt = _widths.begin(); cnt != _widths.end(); cnt++){
                result += repeat(*cnt, _horizontal);
                result += _corner;
            }
            return result;
        }
};

std::ostream& operator<<(std::ostream& stream, TextTable const& table){
    table.setup();
    stream << table.ruler() << "\n";

    // print header
    TextTable::Row const& row = *(table.rows().begin());
    stream << table.vertical();
    for(unsigned i = 0; i< row.size(); i++){
        auto alignment = table.alignment(i) == TextTable::Alignment::LEFT ? std::left : std::right;
        stream << TERM_BLUE << std::setw(table.width(i)) << alignment  <<row[i]<< TERM_RESET;
        stream << table.vertical();
    }
    stream << "\n" << table.ruler() << "\n";

    // print body
    for(auto rowIterator = table.rows().begin() + 1; rowIterator != table.rows().end(); rowIterator++){
        TextTable::Row const& row = *rowIterator;
        stream << table.vertical();
        for(unsigned i = 0; i< row.size(); i++){
            auto alignment = table.alignment(i) == TextTable::Alignment::LEFT ? std::left : std::right;
            stream << std::setw(table.width(i)) << alignment << row[i];
            stream << table.vertical();
        }
        stream << "\n" << table.ruler() << "\n";
    }
    return stream;
}