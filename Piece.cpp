#include <iostream>
#include <cstdint>
#include <sstream>
#include <functional>
#include <vector>
#include <cstring>
#include <initializer_list>

class Piece;

class MultiLine
{
 private:
  std::ostream& m_os;
  int m_lines;
  std::vector<std::stringstream> m_ss;

 private:
  void newline_check()
  {
    std::string const& str(m_ss[0].str());
    if (!str.empty() && str[str.length() - 1] == '\n')
    {
      for (int l = 0; l < m_lines; ++l)
      {
        m_os << m_ss[l].str();
        m_ss[l].str(std::string());
      }
    }
    return;
  }

 public:
  MultiLine(std::ostream& os, int lines) : m_os(os), m_lines(lines), m_ss(lines) { }
  MultiLine(MultiLine& ml) : m_os(ml.m_os), m_lines(ml.m_lines), m_ss(m_lines)
  {
    std::string s(ml.m_ss[0].str().length(), ' ');
    for (int l = 0; l < m_lines; ++l)
      m_ss[l] << s;
  }

  void add(std::function<void(std::ostream&, int)> lambda)
  {
    for (int l = 0; l < m_lines; ++l)
      lambda(m_ss[l], l);
    newline_check();
  }

  MultiLine& operator<<(std::ostream& func(std::ostream&))
  {
    for (int l = 0; l < m_lines; ++l)
      func(m_ss[l]);
    newline_check();
    return *this;
  }

  template<typename T>
  MultiLine& operator<<(T const& data)
  {
    int text_line = m_lines / 2;
    std::stringstream ss;
    ss << data;
    int len = ss.str().length();
    std::string ws(len, ' ');
    for (int l = 0; l < m_lines; ++l)
    {
      if (l == text_line)
        m_ss[l] << ss.str();
      else
        m_ss[l] << ws;
    }
    return *this;
  }
};

enum piece_st
{
  L0, L1, L2, L3,
  M0, M1, M2, M3,
  T0, T1, T2, T3,
  B0, B1, na, E
};

class Piece
{
 public:
  static int constexpr side = 3;

 private:
  static std::string const s_pawn;
  static char const* const s_type2str[4][side][4];
  uint8_t m_type : 2;
  uint8_t m_rotation : 2;
  uint8_t m_pin : 1;

 public:
  Piece() : m_type(3), m_rotation(2), m_pin(0) { }
  Piece(uint8_t type, uint8_t rotation) : m_type(type), m_rotation(rotation) { }
  Piece(Piece const& piece) : m_type(piece.m_type), m_rotation(piece.m_rotation), m_pin(piece.m_pin) { }
  Piece(piece_st p) : m_type(p / 4), m_rotation(p % 4), m_pin(0) { }

  void pin(bool p)
  {
    m_pin = p;
  }

  bool is_hole() const
  {
    return m_type == 3 && m_rotation == 3;
  }

  friend MultiLine& operator<<(MultiLine& ps, Piece const& piece)
  {
    ps.add([piece](std::ostream& os, int line)
        {
          if (!piece.m_pin)
            os << Piece::s_type2str[piece.m_type][line][piece.m_rotation];
          else
          {
            std::string s = Piece::s_type2str[piece.m_type][line][piece.m_rotation];
            std::string::size_type pos = s.find(u8"○", strlen(u8"○"));
            if (pos != std::string::npos)
              s.replace(pos, strlen(u8"○"), Piece::s_pawn);
            os << s;
          }
        });
    return ps;
  }
};

std::string const Piece::s_pawn = "◉";

char const* const Piece::s_type2str[4][Piece::side][4] =
        { { { u8"┏━━━┑", u8"┍━━━┓", u8"┌───┒", u8"┎───┐" },
            { u8"┃ ○ │", u8"│ ○ ┃", u8"│ ○ ┃", u8"┃ ○ │" },
            { u8"┖───┘", u8"└───┚", u8"┕━━━┛", u8"┗━━━┙" } },
          { { u8"┏━━━┑", u8"┍━━━┓", u8"┌───┒", u8"┎───┐" },
            { u8"┃░░░│", u8"│░░░┃", u8"│░░░┃", u8"┃░░░│" },
            { u8"┖───┘", u8"└───┚", u8"┕━━━┛", u8"┗━━━┙" } },
          { { u8"┎▗▄▖┒", u8"┍━━━┑", u8"┎───┒", u8"┍━━━┑" },
            { u8"┃ ○ ┃", u8"│ ○ ▌", u8"┃ ○ ┃", u8"▐▍○ │" },
            { u8"┖───┚", u8"┕━━━┙", u8"┖▝▀▘┚", u8"┕━━━┙" } },
          { { u8"┎───┒", u8"┍━━━┑", u8" ╲ ╱ ", u8"░░░░░" },
            { u8"┃░░░┃", u8"│░░░│", u8"  ╳  ", u8"░░░░░" },
            { u8"┖───┚", u8"┕━━━┙", u8" ╱ ╲ ", u8"░░░░░" } } };

enum dir_nt
{
  up,
  right,
  down,
  left
};

class Move
{
 private:
  bool m_move_pin;
  dir_nt m_dir;

 public:
  Move(dir_nt dir, bool pin = false) : m_move_pin(pin), m_dir(dir) { }

  // Accessor.
  bool move_pin() const { return m_move_pin; }
  dir_nt dir() const { return m_dir; }
};

class Game
{
 private:
  int8_t m_pin_row;
  int8_t m_pin_col;
  int8_t m_hole_row;
  int8_t m_hole_col;
  std::array<std::array<Piece, 3>, 3> m_board;

 public:
  Game(std::initializer_list<piece_st> init) : m_pin_row(-1), m_pin_col(-1), m_hole_row(-1), m_hole_col(-1)
  {
    std::initializer_list<piece_st>::iterator i = init.begin();
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
      {
        m_board[row][col] = *i++;
	if (m_board[row][col].is_hole())
	{
	  m_hole_row = row;
	  m_hole_col = col;
	}
      }
  }

  void pin(int row, int col)
  {
    if (m_pin_row != -1)
      m_board[m_pin_row][m_pin_col].pin(false);
    m_board[row][col].pin(true);
    m_pin_row = row;
    m_pin_col = col;
  }

  void print_to(std::ostream& os) const;

  void move(Move const& move);

  friend std::ostream& operator<<(std::ostream& os, Game const& game)
  {
    game.print_to(os);
    return os;
  }
};

void Game::print_to(std::ostream& os) const
{
  MultiLine ps(os, Piece::side);
  os << std::endl;
  for (int row = 0; row < 3; ++row)
  {
    for (int col = 0; col < 3; ++col)
      ps << m_board[row][col];
    ps << std::endl;
  }
}

void Game::move(Move const& move)
{
  if (move.move_pin())
  {
  }
  else
  {

  }
}

int main()
{
  Game game = { L0, L1, L2, M0, M1, T0, T1, B0, E };
  game.pin(0, 0);

  game.print_to(std::cout);
  Move move(up);
  game.move(move);
  game.print_to(std::cout);
}
