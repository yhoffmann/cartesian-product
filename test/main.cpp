#include "../include/CartesianProduct.hpp"

#include <iostream>
#include <utility>

int main(int argc, char* argv[]) {
  // const std::vector<float> x(std::atoi(argv[1]), 1.0);
  // const std::vector<float> y(std::atoi(argv[1]), 10.0);
  // const std::vector<float> z(std::atoi(argv[1]), 100.0);
  // const std::vector<float> a(std::atoi(argv[1]), 1000.0);
  //
  // CartesianProduct<float, 4> product(a, x, y, z);
  //
  // float sum1 = 0.0;
  // if (std::atoi(argv[2]) == 0) {
  //   std::cout << "Doing product version" << std::endl;
  //   for (const auto& e : product) {
  //     sum1 += e.dim<0>() + e.dim<1>() + e.dim<2>() + e.dim<3>();
  //     std::cout << e.dim<0>() << " " << e.dim<1>() << " " << e.dim<2>() << "
  //     "
  //               << e.idx<0>() << " " << e.idx<1>() << " " << e.idx<2>()
  //               << std::endl;
  //   }
  // }
  //
  // float sum2 = 0.0;
  // if (std::atoi(argv[2]) != 0) {
  //   std::cout << "Doing manual version" << std::endl;
  //   for (const auto& ea : a) {
  //     for (const auto& ex : x) {
  //       for (const auto& ey : y) {
  //         for (const auto& ez : z) {
  //           sum2 += ea + ex + ey + ez;
  //         }
  //         // std::cout << ex << " " << ey << " " << ez << std::endl;
  //       }
  //     }
  //   }
  // }
  //
  // std::cout << sum1 << " " << sum2 << std::endl;

  const std::vector<std::pair<float, float>> eta1Bins{
      {0.0, 0.4}, {0.4, 0.8}, {0.8, 1.1}, {1.1, 1.37}, {1.37, 1.52}};
  const auto eta2Bins(eta1Bins);

  CartesianProduct<std::pair<float, float>, 2> etaBins(eta1Bins, eta2Bins);

  for (const auto& bins : etaBins) {
    std::cout << bins.get<0>().first << "\t" << bins.get<0>().second << "\t,\t"
              << bins.get<1>().first << "\t" << bins.get<1>().second
              << std::endl;
  }

  CartesianProduct<int, 1> test(std::vector<int>{1});
  for (const auto& e : test) {
    std::cout << e.get<0>() << std::endl;
  }
}
