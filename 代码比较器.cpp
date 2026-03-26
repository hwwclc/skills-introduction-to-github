#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

/**
 * @brief 用于存储差异结果的枚举
 */
enum class DiffType {
	EQUAL,
	DELETE, // 在文件1中存在，文件2中不存在
	INSERT  // 在文件2中存在，文件1中不存在
};

/**
 * @brief 结构体：存储单行差异信息
 */
struct DiffLine {
	DiffType type;
	std::string content;
	int lineNum; // 原始行号
};

/**
 * @brief 代码比较器类
 */
class FileComparator {
private:
	std::vector<std::string> lines1;
	std::vector<std::string> lines2;
	std::vector<DiffLine> result;
	
	/**
	 * @brief 读取文件内容到 vector<string>
	 */
	bool readFile(const std::string& filename, std::vector<std::string>& lines) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cerr << "错误: 无法打开文件 " << filename << std::endl;
			return false;
		}
		
		std::string line;
		while (std::getline(file, line)) {
			// 这里可以选择是否去除行尾的 \r (Windows换行符)
			if (!line.empty() && line.back() == '\r') {
				line.pop_back();
			}
			lines.push_back(line);
		}
		file.close();
		return true;
	}
	
	/**
	 * @brief 核心算法：LCS (最长公共子序列) 回溯以生成差异
	 * 
	 * 使用动态规划构建矩阵，然后回溯找出差异路径
	 */
	void computeLCS() {
		int m = lines1.size();
		int n = lines2.size();
		
		// 创建 DP 表
		// 使用 vector<vector<int>> 可能会导致大文件内存溢出，这里仅做演示
		// 生产环境建议使用滚动数组优化空间复杂度
		std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
		
		// 填充 DP 表
		for (int i = 1; i <= m; ++i) {
			for (int j = 1; j <= n; ++j) {
				if (lines1[i - 1] == lines2[j - 1]) {
					dp[i][j] = dp[i - 1][j - 1] + 1;
				} else {
					dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
				}
			}
		}
		
		// 回溯生成差异结果
		int i = m, j = n;
		std::vector<DiffLine> tempResult;
		
		while (i > 0 || j > 0) {
			if (i > 0 && j > 0 && lines1[i - 1] == lines2[j - 1]) {
				// 内容相同
				tempResult.push_back({DiffType::EQUAL, lines1[i - 1], i});
				i--; j--;
			} else if (j > 0 && (i == 0 || dp[i][j - 1] >= dp[i - 1][j])) {
				// 文件2新增 (INSERT)
				tempResult.push_back({DiffType::INSERT, lines2[j - 1], j});
				j--;
			} else {
				// 文件1删除 (DELETE)
				tempResult.push_back({DiffType::DELETE, lines1[i - 1], i});
				i--;
			}
		}
		
		// 反转结果，因为回溯是从后往前的
		std::reverse(tempResult.begin(), tempResult.end());
		result = tempResult;
	}
	
public:
	/**
	 * @brief 比较两个文件
	 * @return 如果比较成功返回 true
	 */
	bool compare(const std::string& file1, const std::string& file2) {
		lines1.clear();
		lines2.clear();
		result.clear();
		
		if (!readFile(file1, lines1)) return false;
		if (!readFile(file2, lines2)) return false;
		
		computeLCS();
		return true;
	}
	
	/**
	 * @brief 打印差异结果到控制台
	 */
	void printDiff() const {
		std::cout << "--- 差异报告 ---" << std::endl;
		
		for (const auto& line : result) {
			switch (line.type) {
			case DiffType::DELETE:
				std::cout << "- [" << std::setw(3) << line.lineNum << "] " << line.content << std::endl;
				break;
			case DiffType::INSERT:
				std::cout << "+ [" << std::setw(3) << line.lineNum << "] " << line.content << std::endl;
				break;
			case DiffType::EQUAL:
				// 如果需要显示上下文，可以取消注释下面这行
				// std::cout << "  [" << std::setw(3) << line.lineNum << "] " << line.content << std::endl;
				break;
			}
		}
	}
	
	/**
	 * @brief 获取差异统计
	 */
	void printStats() const {
		int adds = 0, dels = 0;
		for(const auto& l : result) {
			if(l.type == DiffType::INSERT) adds++;
			if(l.type == DiffType::DELETE) dels++;
		}
		std::cout << "\n统计: " << adds << " 行新增, " << dels << " 行删除." << std::endl;
	}
};

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "用法: " << argv[0] << " <文件1路径> <文件2路径>" << std::endl;
		return 1;
	}
	
	std::string file1 = argv[1];
	std::string file2 = argv[2];
	
	FileComparator comparator;
	
	std::cout << "正在比较: " << file1 << " 和 " << file2 << "..." << std::endl;
	
	if (comparator.compare(file1, file2)) {
		comparator.printDiff();
		comparator.printStats();
	} else {
		std::cerr << "比较失败。" << std::endl;
		return 1;
	}
	
	return 0;
}
