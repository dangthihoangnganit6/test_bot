#include "ott_strategy_sdk.hpp"
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

// Hàm phụ trợ: Chuyển tọa độ xâu "a1" - "i9" thành tọa độ (col: 0-8, row: 0-8)
pair<int, int> parsePos(const string& pos) {
    int col = pos[0] - 'a';
    int row = pos[1] - '1';
    return {col, row};
}

// Tính khoảng cách Chebyshev (số bước đi tối thiểu trên bàn cờ 8 hướng)
int getDistance(const string& from, const string& to) {
    auto p1 = parsePos(from);
    auto p2 = parsePos(to);
    return max(abs(p1.first - p2.first), abs(p1.second - p2.second));
}

// Kiểm tra typeA có ăn được typeB hay không (dam > keo > la > dam)
bool canCaptureType(const string& typeA, const string& typeB) {
    if (typeA == "dam" && typeB == "keo") return true;
    if (typeA == "keo" && typeB == "la") return true;
    if (typeA == "la" && typeB == "dam") return true;
    return false;
}

Move chooseMove(const GameState& state, const vector<Move>& legalMoves) {
    if (legalMoves.empty()) return Move();

    const string targetPos = (state.player == 'A') ? "i9" : "a1";

    // 1. Kiểm tra nước thắng ngay lập tức
    for (const Move& m : legalMoves) {
        if (m.to == targetPos) {
            return m;
        }
    }

    Move bestMove = legalMoves.front();
    double bestScore = -1e9;

    for (const Move& m : legalMoves) {
        double score = 0.0;

        // Tìm thông tin quân ta đang đi
        string myPieceType = "";
        for (const auto& p : state.pieces) {
            if (p.id == m.piece_id) {
                myPieceType = p.type;
                break;
            }
        }

        // Ưu tiên 1: Ăn quân đối phương
        if (m.has_capture) {
            score += 500.0;
        }

        // Ưu tiên 2: Rút ngắn khoảng cách tới ô đích để chiến thắng
        int distBefore = getDistance(m.from, targetPos);
        int distAfter = getDistance(m.to, targetPos);
        int progress = distBefore - distAfter; // > 0 là đang tiến lại gần
        score += progress * 40.0;

        // Ưu tiên 3: Né ô bị quân địch có thể ăn ngay ở lượt sau
        auto myTargetCoord = parsePos(m.to);
        for (const auto& enemy : state.pieces) {
            if (enemy.player != state.player) {
                // Nếu địch khắc chế được loại quân của ta
                if (canCaptureType(enemy.type, myPieceType)) {
                    auto enemyCoord = parsePos(enemy.pos);
                    int distToEnemy = max(abs(enemyCoord.first - myTargetCoord.first), 
                                          abs(enemyCoord.second - myTargetCoord.second));
                    // Nếu sau khi nhảy tới ô m.to mà nằm ngay tầm đi 1 ô của địch
                    if (distToEnemy <= 1) {
                        score -= 300.0; // Bị đe dọa, trừ điểm nặng
                    }
                }
            }
        }

        // Cập nhật nước đi có điểm đánh giá cao nhất
        if (score > bestScore) {
            bestScore = score;
            bestMove = m;
        }
    }

    return bestMove;
}