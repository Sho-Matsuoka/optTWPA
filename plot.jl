# ==== スクリプト本体 ====
using DelimitedFiles   # テキストファイル読み込み用
using Plots            # プロット用

# ファイルからデータを読み込む
# x.txt, y.txt はそれぞれ 1 列の数値データを想定
x = readdlm("freq_gain_sim.txt") |> vec
y = readdlm("trans_gain_sim.txt") |> vec

# データ長をチェック（オプション）
if length(x) != length(y)
    error("There is a difference between x and y. ", length(x), " ≠ ", length(y))
end

# プロット
plt = plot(
    x, y,
    xlabel = "Frequency [GHz]",
    ylabel = "Gain [dB]",
    legend = false,
    seriestype = :line,    # 線グラフ専用で描く
    marker      = :none,   # マーカー（点）を完全にオフ
    line   = (:solid, 2),
)

# 画面表示
display(plt)

# ファイルに保存する場合
savefig(plt, "gain.png")