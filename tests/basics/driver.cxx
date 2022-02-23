#include <libstud/optional.hxx>

#include <string>
#include <vector>
#include <utility> // move()

#undef NDEBUG
#include <cassert>

using namespace std;

struct move_only
{
  move_only () = default;

  move_only (move_only&&) = default;
  move_only& operator= (move_only&&) = default;

  move_only (const move_only&) = delete;
  move_only& operator= (const move_only&) = delete;
};

int
main ()
{
  using stud::optional;

  {
    using ostr = optional<string>;

    ostr s ("small");
    ostr l ("large large large large large large large large");

    ostr sc (s); assert (sc == s);
    ostr lc (l); assert (lc == l);

    ostr sm (move (sc)); assert (sm == s);
    ostr lm (move (lc)); assert (lm == l);
  }

  {
    optional<move_only> r;
    vector<optional<move_only>> rs;
    rs.emplace_back (move (r));
  }
}
