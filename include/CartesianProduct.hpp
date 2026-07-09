#ifndef CARTESIAN_PRODUCT_HPP_
#define CARTESIAN_PRODUCT_HPP_

#include <array>
#include <vector>

/**
 * @brief Class that allows for iteration over cartesian-product combinations of
 * differently sized vectors of the same type. The idea is that one can use a
 * single C++ iterator to iterate over all combinations instead of writing `N`
 * different for loops
 *
 * @param T The type that the vectors hold
 * @param N The number of vectors to combine (dimension)
 */
template <typename T, std::size_t N>
class CartesianProduct {
 public:
  using Vec = const std::vector<T>*;

  /**
   * @brief Construct from the different vectors differently
   *
   * @param vecs The vecs to build combinations from. The number of vecs to
   * supply needs to match `N`, of course
   *
   * @note The vecs are not copied. The class just saves pointers to them.
   *
   * @attention Make sure to not construct this from temporaries as this will
   * lead to undefined behaviour. TODO: Fix this at some point
   */
  template <typename... Vecs>
  explicit CartesianProduct(const Vecs&... vecs) {
    static_assert(sizeof...(Vecs) == N, "Wrong number of vectors passed.");
    static_assert(sizeof...(Vecs) > 0, "Construct from at least one vector.");

    static_assert((std::is_same_v<Vecs, std::vector<T>> && ...),
                  "All arguments must be std::vector<T>.");

    assign<0>(vecs...);
  }

  /**
   * @brief Class that is returned by the iteration. Gives access to the
   * elements of the vectors that make up the current combination.
   */
  class Entry {
   private:
    friend class CartesianProduct;
    Entry(const std::array<Vec, N>* containers,
          const std::array<std::size_t, N>* indices)
        : m_containers(containers), m_indices(indices) {}

   public:
    /**
     * @brief Returns element in combination
     *
     * @param IDX the dimension (example below)
     *
     * @returns Element of vector used in the combination
     *
     * @code{.cpp}
     * std::vector<int> x{1, 2};
     * std::vector<int> y{10, 20};
     * CartesianProduct<int, 2> prod(x, y);
     * for (const auto& combination : prod)
     *   std::cout << combination.dim<0>() << "\n"; // this will return the
     *                                              // values in `x` in the
     *                                              // current combination
     * @endcode
     *
     * @note Does bounds checking at compile time. This, of course, means that
     * we cannot supply run-time values. Use @see T&dim(const std::size_t idx)
     * for that instead
     */
    template <std::size_t IDX>
    const T& get() const {
      static_assert(IDX < N, "Dimension out of bounds.");

      return (*(*m_containers)[IDX])[(*m_indices)[IDX]];
    }

    /**
     * @brief Non-templated version of @see T& dim()
     *
     * @param idx the dimension (@see T& dim())
     *
     * @returns Element of vector used in the combination
     */
    const T& get(const std::size_t idx) {
      return (*(*m_containers)[idx])[(*m_indices)[idx]];
    }

    /*
     * @brief Returns index of element in combination
     *
     * @param IDX the dimension (@see T& dim())
     *
     * @returns Index of element of vector used in the combination */
    template <std::size_t IDX>
    std::size_t idx() const {
      static_assert(IDX < N, "Dimension out of bounds.");

      return (*m_indices)[IDX];
    }

    /**
     * @brief Non-templated version of @see std::site_t idx()
     *
     * @param IDX the dimension (@see T& dim())
     *
     * @returns Index of element of vector used in the combination
     *
     * @attention Does not do bounds checking, neither at compile time nor at
     * run time. Is also slower than @see std::size_t idx() */
    std::size_t idx(const std::size_t idx) { return (*m_indices)[idx]; }

   private:
    const std::array<Vec, N>* m_containers;
    const std::array<std::size_t, N>* m_indices;
  };

  /**
   * @brief Class the handles the iteration over combinations of entries from
   * the vectors.
   */
  class Iterator {
   public:
    Iterator(const std::array<Vec, N>* containers, bool end = false)
        : m_containers(containers), m_isFinished(end) {
      m_indices.fill(0);

      if (m_isFinished) return;

      for (auto ptr : *m_containers) {
        if (ptr->empty()) {
          m_isFinished = true;
          return;
        }
      }
    }

    Entry operator*() const { return Entry(m_containers, &m_indices); }

    Iterator& operator++() {
      for (std::size_t d = N; d-- > 0;) {
        ++m_indices[d];

        if (m_indices[d] < (*m_containers)[d]->size()) return *this;

        m_indices[d] = 0;
      }

      m_isFinished = true;

      return *this;
    }

    bool operator==(const Iterator& other) const {
      if (m_isFinished && other.m_isFinished) return true;

      if (m_isFinished != other.m_isFinished) return false;

      return m_indices == other.m_indices;
    }

    bool operator!=(const Iterator& other) const { return !(*this == other); }

   private:
    const std::array<Vec, N>* m_containers;
    std::array<std::size_t, N> m_indices;
    bool m_isFinished;
  };

  Iterator begin() const { return Iterator(&m_containers); }

  Iterator end() const { return Iterator(&m_containers, true); }

 private:
  template <std::size_t I, typename First, typename... Rest>
  void assign(const First& first, const Rest&... rest) {
    m_containers[I] = &first;

    if constexpr (sizeof...(Rest) > 0) assign<I + 1>(rest...);
  }

  std::array<Vec, N> m_containers;
};  // class CartesianProduct

#endif  // CARTESIAN_PRODUCT_HPP_
