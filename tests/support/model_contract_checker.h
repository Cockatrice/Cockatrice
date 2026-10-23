#ifndef COCKATRICE_TEST_MODEL_CONTRACT_CHECKER_H
#define COCKATRICE_TEST_MODEL_CONTRACT_CHECKER_H

#include <QAbstractItemModel>
#include <QAbstractItemModelTester>
#include <memory>

namespace test_support
{
/**
 * @brief Keeps a QAbstractItemModelTester alive for the lifetime of a scope.
 *
 * The tester only observes the model while it exists, so construct this before
 * the mutations a test wants to validate and keep it in scope until the end.
 * Fatal reporting mode aborts on the first contract violation; a warning would
 * let the test pass while printing a line nobody reads in CI.
 */
class ModelContractChecker
{
public:
    explicit ModelContractChecker(QAbstractItemModel *model)
        : tester(
              std::make_unique<QAbstractItemModelTester>(model, QAbstractItemModelTester::FailureReportingMode::Fatal))
    {
    }

    ModelContractChecker(const ModelContractChecker &) = delete;
    ModelContractChecker &operator=(const ModelContractChecker &) = delete;

private:
    std::unique_ptr<QAbstractItemModelTester> tester;
};
} // namespace test_support

#endif // COCKATRICE_TEST_MODEL_CONTRACT_CHECKER_H
