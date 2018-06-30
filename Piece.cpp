#include <iostream>
#include <cstdint>
#include <sstream>
#include <functional>
#include <vector>
#include <cstring>
#include <initializer_list>
#include <cassert>
#include <set>
#include <list>

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

enum dir_nt
{
  up,
  right,
  left,
  down
};

std::ostream& operator<<(std::ostream& os, dir_nt dir)
{
  switch (dir)
  {
    case up:
      os << "up";
      break;
    case right:
      os << "right";
      break;
    case left:
      os << "left";
      break;
    case down:
      os << "down";
      break;
  }
  return os;
}

dir_nt inverse(dir_nt dir)
{
  return (dir_nt)(3 - dir);
}

class Pos
{
 private:
  int8_t m_row : 4;
  int8_t m_col : 4;

 public:
  Pos() : m_row(-1), m_col(-1) { }
  Pos(int row, int col) : m_row(row), m_col(col) { }
  Pos& operator=(Pos const& pos) { m_row = pos.m_row; m_col = pos.m_col; return *this; }

  bool is_valid() const { return m_row != -1 && m_col != -1; }
  bool is_solution() const { return m_row == 0 && m_col == -1; }
  int8_t row() const { return m_row; }
  int8_t col() const { return m_col; }
  bool is_edge(dir_nt dir) const
  {
    switch (dir)
    {
      case up:
        return m_row == 0;
      case right:
        return m_col == 2;
      case left:
        return m_col == 0;
      case down:
        return m_row == 2;
    }
    return false;
  }

  Pos step(dir_nt dir) const
  {
    Pos pos(m_row, m_col);
    switch (dir)
    {
      case up:
	--pos.m_row;
	break;
      case right:
	++pos.m_col;
	break;
      case left:
	--pos.m_col;
	break;
      case down:
	++pos.m_row;
	break;
    }
    assert(0 <= pos.m_row && pos.m_row <= 2 && 0 <= pos.m_col && pos.m_col <= 2);
    return pos;
  }

  friend std::ostream& operator<<(std::ostream& os, Pos const& pos)
  {
    os << '(' << (int)pos.m_row << ", " << (int)pos.m_col << ')';
    return os;
  }

  bool operator!=(Pos const& pos) const
  {
    return m_row != pos.m_row || m_col != pos.m_col;
  }

  bool operator<(Pos const& pos) const
  {
    return m_row < pos.m_row || (m_row == pos.m_row && m_col < pos.m_col);
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
  static int const s_dir_sum[4][4];
  static int const s_level[4][4][4];
  uint8_t m_type : 2;
  uint8_t m_rotation : 2;
  uint8_t m_pin : 1;

 public:
  Piece() : m_type(3), m_rotation(2), m_pin(0) { }
  Piece(uint8_t type, uint8_t rotation) : m_type(type), m_rotation(rotation) { }
  Piece(Piece const& piece) : m_type(piece.m_type), m_rotation(piece.m_rotation), m_pin(piece.m_pin) { }
  Piece(piece_st p) : m_type(p / 4), m_rotation(p % 4), m_pin(0) { }

  uint8_t val() const
  {
    return m_type * 4 + m_rotation;
  }

  void pin(bool p)
  {
    m_pin = p;
  }

  bool is_hole() const
  {
    return m_type == 3 && m_rotation == 3;
  }

  bool is_pin() const
  {
    return m_pin;
  }

  int level(dir_nt dir) const
  {
    return s_level[m_type][m_rotation][dir];
  }

  int level() const
  {
    return m_type & 1;
  }

  dir_nt other(dir_nt dir) const
  {
    return (dir_nt)(s_dir_sum[m_type][m_rotation] - dir);
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
        { { { u8"┎───┐", u8"┏━━━┑", u8"┍━━━┓", u8"┌───┒" },
            { u8"┃ ○ │", u8"┃ ○ │", u8"│ ○ ┃", u8"│ ○ ┃" },
            { u8"┗━━━┙", u8"┖───┘", u8"└───┚", u8"┕━━━┛" } },
          { { u8"┎───┐", u8"┏━━━┑", u8"┍━━━┓", u8"┌───┒" },
            { u8"┃░░░│", u8"┃░░░│", u8"│░░░┃", u8"│░░░┃" },
            { u8"┗━━━┙", u8"┖───┘", u8"└───┚", u8"┕━━━┛" } },
          { { u8"┎▗▄▖┒", u8"┍━━━┑", u8"┎───┒", u8"┍━━━┑" },
            { u8"┃ ○ ┃", u8"│ ○ ▌", u8"┃ ○ ┃", u8"▐▍○ │" },
            { u8"┖───┚", u8"┕━━━┙", u8"┖▝▀▘┚", u8"┕━━━┙" } },
          { { u8"┎───┒", u8"┍━━━┑", u8" ╲ ╱ ", u8"░░░░░" },
            { u8"┃░░░┃", u8"│░░░│", u8"  ╳  ", u8"░░░░░" },
            { u8"┖───┚", u8"┕━━━┙", u8" ╱ ╲ ", u8"░░░░░" } } };

// Directions encoded as
//     0
//  2     1
//     3
//static
int const Piece::s_dir_sum[4][4] =
        { { 1, 4, 5, 2 },
          { 1, 4, 5, 2 },
          { 3, 3, 3, 3 },
          { 3, 3, 0, 0 } };

//static
int const Piece::s_level[4][4][4] =
        { { { 0, 0, 2, 2 }, { 2, 0, 2, 0 }, { 2, 2, 0, 0 }, { 0, 2, 0, 2 } },
          { { 1, 1, 2, 2 }, { 2, 1, 2, 1 }, { 2, 2, 1, 1 }, { 1, 2, 1, 2 } },
          { { 1, 2, 2, 0 }, { 2, 1, 0, 2 }, { 0, 2, 2, 1 }, { 2, 0, 1, 2 } },
          { { 1, 2, 2, 1 }, { 2, 1, 1, 2 }, { 0, 0, 0, 0 }, { 2, 2, 2, 2 } } };

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
  mutable std::list<Game>::iterator m_parent;
  std::array<std::array<Piece, 3>, 3> m_board;
  Pos m_pin;
  Pos m_hole;
  mutable bool m_parent_set;
  mutable int m_generation;
  static int s_generation;

 public:
  Game(std::initializer_list<piece_st> init) : m_parent_set(false), m_generation(0)
  {
    std::initializer_list<piece_st>::iterator i = init.begin();
    for (int row = 0; row < 3; ++row)
      for (int col = 0; col < 3; ++col)
      {
        m_board[row][col] = *i++;
	if (m_board[row][col].is_hole())
	{
	  m_hole = Pos(row, col);
	}
      }
  }
  Game(Game const& game) : m_parent(game.m_parent), m_board(game.m_board), m_pin(game.m_pin), m_hole(game.m_hole), m_parent_set(game.m_parent_set), m_generation(game.m_generation) { }
  ~Game() { m_parent_set = false; }

  void set_parent(std::list<Game>::iterator parent) const { m_parent = parent; m_generation = s_generation++; m_parent_set = true; }
  std::list<Game>::iterator get_parent() const { assert(m_parent_set); return m_parent; }
  int get_generation() const { return m_generation; }

  void pin(int row, int col)
  {
    if (m_pin.is_valid())
      m_board[m_pin.row()][m_pin.col()].pin(false);
    m_board[row][col].pin(true);
    m_pin = Pos(row, col);
    assert(piece_at(m_pin).level() == 0);
  }

  void print_to(std::ostream& os) const;

  Game move(Move const& move) const;

  Piece& piece_at(Pos pos) { return m_board[pos.row()][pos.col()]; }
  Piece const& piece_at(Pos pos) const { return m_board[pos.row()][pos.col()]; }

  Pos cross_from(Pos pos, dir_nt dir) const
  {
    if (!pos.is_edge(dir))
    {
      int lvl = piece_at(pos).level(dir);
      Pos res = pos.step(dir);
      if (lvl != 2 && lvl == piece_at(res).level(inverse(dir)))
        return res;
    }
    else if (dir == left && pos.row() == 0 && piece_at(pos).level(dir) == 1)
      return {0, -1};
    return {};
  }

  Pos move_pin(dir_nt dir) const
  {
    Pos pos(m_pin);
    do
    {
      Pos save(pos);
      pos = cross_from(pos, dir);
      if (!pos.is_valid())
      {
        //std::cout << "  cross_from(" << save << ", " << dir << ") returns an invalid position." << std::endl;
        break;
      }
      //else
      //  std::cout << "  cross_from(" << save << ", " << dir << ") returns " << pos << std::endl;
      dir = piece_at(pos).other(inverse(dir));
    }
    while (piece_at(pos).level() == 1);
    return pos;
  }

  bool generate(std::vector<Move>& moves)
  {
    for (int d = 0; d < 4; ++d)
    {
      dir_nt dir = (dir_nt)d;
      if (!m_hole.is_edge(dir) && !piece_at(m_hole.step(dir)).is_pin())
        moves.emplace_back(dir, false);
      Pos pos = move_pin(dir);
      if (pos.is_valid())
        moves.emplace_back(dir, true);
      else if (pos.is_solution())
      {
        std::cout << "\nMove pin " << dir << " from," << std::endl;
        return true;
      }
    }
    return false;
  }

  bool add_new_boards(std::set<Game>& old_boards, std::list<Game>& new_boards, std::list<Game>::iterator parent);

  friend std::ostream& operator<<(std::ostream& os, Game const& game)
  {
    game.print_to(os);
    return os;
  }

  bool operator<(Game const& game) const
  {
    if (m_pin != game.m_pin)
      return m_pin < game.m_pin;
    for (int row = 0; row < 3; ++row)
    {
      for (int col = 0; col < 3; ++col)
      {
        int val1 = m_board[row][col].val();
        int val2 = game.m_board[row][col].val();
        if (val1 != val2)
          return val1 < val2;
      }
    }
    // Equal.
    return false;
  }
};

//static
int Game::s_generation;

void Game::print_to(std::ostream& os) const
{
  MultiLine ps(os, Piece::side);
  os << std::endl;
  if (m_parent_set)
  {
    if (get_generation() > 0)
      os << m_parent->get_generation() << " --> " << get_generation() << std::endl;
    else
      os << get_generation() << std::endl;
  }
  for (int row = 0; row < 3; ++row)
  {
    for (int col = 0; col < 3; ++col)
      ps << m_board[row][col];
    ps << std::endl;
  }
}

Game Game::move(Move const& move) const
{
  Game game{*this};
  Pos& pos(move.move_pin() ? game.m_pin : game.m_hole);
  Pos piece;
  if (!move.move_pin())
  {
    piece = pos.step(move.dir());
    std::swap(game.m_board[pos.row()][pos.col()], game.m_board[piece.row()][piece.col()]);
  }
  else
  {
    piece = move_pin(move.dir());
    game.pin(piece.row(), piece.col());
  }
  pos = piece;
  return game;
}

bool Game::add_new_boards(std::set<Game>& old_boards, std::list<Game>& new_boards, std::list<Game>::iterator parent)
{
  std::vector<Move> moves;
  if (generate(moves))
    return true;

  for (auto&& mv : moves)
  {
    Game b2{move(mv)};
    auto res = old_boards.insert(b2);
    if (res.second)
    {
      //std::cout << "INSERTED: " << *res.first << std::endl;
      res.first->set_parent(parent);
      new_boards.push_back(*res.first);
    }
#if 0
    else
    {
      std::cout << "NOT INSERTED: " << b2 << " because already found " << *res.first << std::endl;
      std::cout << "FORMER < LATTER : " << (b2 < *res.first) << std::endl;
      std::cout << "LATTER < FORMER : " << (*res.first < b2) << std::endl;
    }
#endif
  }

  return false;
}

int main()
{
  Game game = {
    B0, L1, M2,
    T3, L3, E,
    L0, M0, B1
  };
  game.pin(1, 0);

  std::set<Game> all_boards;
  std::list<Game> new_boards;
  new_boards.push_back(game);
  std::list<Game>::iterator current = new_boards.begin();
  std::list<Game>::iterator last = current;
  current->set_parent(new_boards.end());
  current->print_to(std::cout);
  int depth = 0;
  while (current != new_boards.end())
  {
    if (/*depth < 2 &&*/ current->add_new_boards(all_boards, new_boards, current))
      break;
    if (current == last)
    {
      ++depth;
      last = new_boards.end();
      --last;
    }
    ++current;
  }
  std::cout << "Found " << all_boards.size() << " different reachable positions, with maximum depth of " << depth << '.' << std::endl;
  if (current != new_boards.end())
  {
    int moves = -1;
    do
    {
      ++moves;
      current->print_to(std::cout);
      current = current->get_parent();
    }
    while (current != new_boards.end());
    std::cout << "Found solution of " << moves << " moves." << std::endl;
  }
  else
  {
    std::cout << "No solution found." << std::endl;
    for (auto&& game : all_boards)
      game.print_to(std::cout);
  }
}
