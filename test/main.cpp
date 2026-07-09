#include "../include/CartesianProduct.hpp"

#include <iostream>
#include <utility>

int main(int argc, char* argv[]) {
  const std::vector<float> x(std::atoi(argv[1]), 1.0);
  const std::vector<float> y(std::atoi(argv[1]), 10.0);
  const std::vector<float> z(std::atoi(argv[1]), 100.0);
  const std::vector<float> a(std::atoi(argv[1]), 1000.0);

  CartesianProduct product(a, x, y, z);

  float sum1 = 0.0;
  if (std::atoi(argv[2]) == 0) {
    std::cout << "Doing product version" << std::endl;
    for (const auto& e : product) {
      sum1 += e.get<0>() + e.get<1>() + e.get<2>() + e.get<3>();
      // std::cout << e.get<0>() << " " << e.get<1>() << " " << e.get<2>() << "
      // "
      //           << e.idx<0>() << " " << e.idx<1>() << " " << e.idx<2>()
      //           << std::endl;
    }
  }

  float sum2 = 0.0;
  if (std::atoi(argv[2]) != 0) {
    std::cout << "Doing manual version" << std::endl;
    for (const auto& ea : a) {
      for (const auto& ex : x) {
        for (const auto& ey : y) {
          for (const auto& ez : z) {
            sum2 += ea + ex + ey + ez;
          }
          // std::cout << ex << " " << ey << " " << ez << std::endl;
        }
      }
    }
  }

  std::cout << sum1 << " " << sum2 << " after " << double(product.size())
            << " iterations" << std::endl;

  // const std::vector<std::pair<float, float>> eta1Bins{
  //     {0.0, 0.4}, {0.4, 0.8}, {0.8, 1.1}, {1.1, 1.37}, {1.37, 1.52}};
  // const auto eta2Bins(eta1Bins);
  //
  // CartesianProduct<std::pair<float, float>, std::pair<float, float>> etaBins(
  //     eta1Bins, eta2Bins);
  //
  // for (const auto& bins : etaBins) {
  //   // std::cout << bins.get<0>().first << "\t" << bins.get<0>().second <<
  //   "\t,\t"
  //   //           << bins.get<1>().first << "\t" << bins.get<1>().second
  //   //           << "\tidcs:\t" << bins.idx<0>() << "\t" << bins.idx<1>()
  //   //           << std::endl;
  // }
  // std::cout << "size is " << etaBins.size() << std::endl;
  //
  // auto x = std::vector<int>(6, 0);
  // auto y = std::vector<int>(7, 1);
  // auto z = std::vector<int>(7, 2);
  // CartesianProduct test(x, y, z);
  // for (const auto& e : test) {
  //   // std::cout << e.get<0>() << std::endl;
  // }
  // std::cout << "size is " << test.size() << std::endl;
}
