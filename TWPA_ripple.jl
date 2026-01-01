using JosephsonCircuits
using Plots
using Statistics

# リップル計算用の補助関数
function calculate_ripple(freqs, gains, range_ghz)
    # 特定の帯域（例：4GHz〜12GHz）のデータを抽出
    mask = (freqs .>= range_ghz[1]) .& (freqs .<= range_ghz[2])
    target_gains = gains[mask]
    if isempty(target_gains) return 0.0 end
    
    # 簡易的なリップル定義：最大値 - 最小値（または平均からの偏差）
    return maximum(target_gains) - minimum(target_gains)
end

function compare_resolution()
    # --- パラメータ設定 (ユーザーの値を流用) ---
    # (ここでは簡略化のため、回路構築部分は共通変数として扱う想定です)
    
    # 周波数定義
    f_start, f_end = 1.0, 16.0
    ws_coarse = 2π * (f_start:0.1:f_end) * 1e9
    ws_fine   = 2π * (f_start:0.01:f_end) * 1e9

    # シミュレーション実行 (粗い方)
    println("Running coarse simulation (0.1 GHz step)...")
    rpm_coarse = hbsolve(ws_coarse, wp, sources, Nmod, Npump, circuit, circuitdefs)
    gains_coarse = 10 .* log10.(abs2.(rpm_coarse.linearized.S((0,), 2, (0,), 1, :)))

    # シミュレーション実行 (細かい方)
    println("Running fine simulation (0.01 GHz step)...")
    rpm_fine = hbsolve(ws_fine, wp, sources, Nmod, Npump, circuit, circuitdefs)
    gains_fine = 10 .* log10.(abs2.(rpm_fine.linearized.S((0,), 2, (0,), 1, :)))

    # --- 定量評価 ---
    eval_range = (4.0, 12.0) # 評価する周波数範囲
    ripple_c = calculate_ripple(ws_coarse ./ (2π*1e9), gains_coarse, eval_range)
    ripple_f = calculate_ripple(ws_fine ./ (2π*1e9), gains_fine, eval_range)

    println("-"^30)
    println("Ripple (0.1 GHz step):  $(round(ripple_c, digits=4)) dB")
    println("Ripple (0.01 GHz step): $(round(ripple_f, digits=4)) dB")
    println("Underestimation Ratio:  $(round(ripple_f / ripple_c, digits=2))x")
    println("-"^30)

    # --- プロット ---
    p = plot(ws_fine ./ (2π*1e9), gains_fine, label="Fine (0.01GHz)", alpha=0.8, color=:blue)
    plot!(ws_coarse ./ (2π*1e9), gains_coarse, label="Coarse (0.1GHz)", 
          marker=:circle, markersize=2, alpha=0.6, color=:red, linestyle=:dash)
    
    xlabel!("Frequency (GHz)")
    ylabel!("Gain (dB)")
    title!("Resolution Comparison: Ripple Visibility")
    
    display(p)
end