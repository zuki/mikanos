/**
 * @file memory_manager.hpp
 *
 * メモリ管理クラスと周辺機能を集めたファイル。
 */

#pragma once

#include <array>
#include <limits>

#include "error.hpp"

namespace {
    constexpr unsigned long long operator""_KiB(unsigned long long kib) {
        return kib * 1024;
    }

    constexpr unsigned long long operator""_MiB(unsigned long long mib) {
        return mib * 1024_KiB;
    }

    constexpr unsigned long long operator""_GiB(unsigned long long gib) {
        return gib * 1024_MiB;
    }
}

/** @brief 物理メモリフレーム1つの大きさ（バイト） */
static const auto kBytesPerFrame{4_KiB};

class FrameID {
public:
    explicit FrameID(size_t id) : id_{id} {}
    size_t ID() const { return id_; }
    void *Frame() const { return reinterpret_cast<void *>(id_ * kBytesPerFrame); }

private:
    size_t id_;
};

static const FrameID kNullFrame{std::numeric_limits<size_t>::max()};

/** @brief ビットマップ配列を用いてフレーム単位でメモリ管理するクラス
 *
 * 1ビットを1フレームに対応させて、ビットマップにより空きフレームを管理する。
 * 配列alloc_mapの各ビットがフレームに対応し、0なら空き、1なら使用中。
 * alloc_map[n]のmビットめが対応する物理アドレスは次の式でも止まる。
 *
 *   kBytesPerFrame * (n * kBitsPerMapLine + m)
 */

class BitmapMemoryManager {
public:
    /** @brief このメモリ管理クラスで扱える最大メモリ量（バイト） */
    static const auto kMaxPhysicalMemoryBytes{128_GiB};

    /** @brief kMaxPhysicalMemoryBytesまでの物理メモリを扱うために必要なフレーム数 */
    static const auto kFrameCount{kMaxPhysicalMemoryBytes / kBytesPerFrame};

    /** @brief ビットマップ配列の要素型 */
    using MapLineType = unsigned long;

    /** @brief ビットマップ配列の1つの要素のビット数 == フレーム数 */
    static const size_t kBitsPerMapLine{8 * sizeof(MapLineType)};

    /** @brief インスタンスを初期化する */
    BitmapMemoryManager();

    /** @brief 要求されたフレーム数の領域を確保して戦闘のフレームIDを返す */
    WithError<FrameID> Allocate(size_t num_frames);
    Error Free(FrameID start_frame, size_t num_frames);
    void MarkAllocated(FrameID start_frame, size_t num_frames);

    /** @brief このメモリマネージャで扱うメモリ範囲を設定する
     * この呼び出し以降、Allocateによるメモリ割り当ては設定された範囲内でのい行われる.
     *
     * @param range_begin メモリ範囲の始点
     * @param range_end   メモリ範囲の終点（最終フレームの次のフレーム）
     */
    void SetMemoryRange(FrameID range_bebin, FrameID range_end);

private:
    std::array<MapLineType, kFrameCount / kBitsPerMapLine> alloc_map_;
    /** @brief このメモリマネージャで扱うメモリ範囲の始点 */
    FrameID range_begin_;
    /** @breif このメモリマネージャで扱うメモリ範囲の終点（最終フレームの次のフレーム） */
    FrameID range_end_;

    bool GetBit(FrameID frame) const;
    void SetBit(FrameID frame, bool allocated);
};
