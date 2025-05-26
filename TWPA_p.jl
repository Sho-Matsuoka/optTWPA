using Distributed
# 必要に応じてプロセス数を追加
addprocs()

# ワーカー上では JosephsonCircuits だけをロードしてシミュレーション関数を定義
@everywhere using JosephsonCircuits
@everywhere function simulate_chunk(ws_chunk, wp, sources, Nmod, Npump, circuit, defs)
    hb = hbsolve(ws_chunk, wp, sources, Nmod, Npump, circuit, defs)
    return ws_chunk, hb
end

# マスター上でのみ使うパッケージ
using DelimitedFiles, ModelingToolkit, Plots

# 回路構築
function build_circuit()
    @variables Rleft Rright Cg Lj Cj Cc Cr Lr Cn
    circ = Tuple{String,String,String,Num}[]
    # 入出力ポートと最初の要素
    push!(circ, ("P$(1)_$(0)", "1", "0", 1))
    push!(circ, ("R$(1)_$(0)", "1", "0", Rleft))
    push!(circ, ("C$(1)_$(0)", "1", "0", Cg))
    push!(circ, ("Lj$(1)_$(2)", "1", "2", Lj))
    push!(circ, ("C$(1)_$(2)", "1", "2", Cj))

    Nj = 2400
    pmrpitch = 3
    j = 2
    for i in 2:Nj-1
        if mod(i, pmrpitch) == (pmrpitch ÷ 3)
            push!(circ, ("C$(j)_$(0)", "$(j)", "0", Cn))
            push!(circ, ("Lj$(j)_$(j+2)", "$(j)", "$(j+2)", Lj))
            push!(circ, ("C$(j)_$(j+2)", "$(j)", "$(j+2)", Cj))
            push!(circ, ("C$(j)_$(j+1)", "$(j)", "$(j+1)", Cc))
            push!(circ, ("C$(j+1)_$(0)", "$(j+1)", "0", Cr))
            push!(circ, ("L$(j+1)_$(0)", "$(j+1)", "0", Lr))
            j += 1
        else
            push!(circ, ("C$(j)_$(0)", "$(j)", "0", Cg))
            push!(circ, ("Lj$(j)_$(j+1)", "$(j)", "$(j+1)", Lj))
            push!(circ, ("C$(j)_$(j+1)", "$(j)", "$(j+1)", Cj))
        end
        j += 1
    end

    # 出力ポートと終端抵抗
    push!(circ, ("R$(j)_$(0)", "$(j)", "0", Rright))
    push!(circ, ("P$(j)_$(0)", "$(j)", "0", 2))

    defs = Dict(
        Lj => IctoLj(6.4e-6),
        Cg => 25.0e-15,
        Cc => 18.0e-15,
        Cn => 6.001e-15,
        Cr => 5.00e-12,
        Lr => 65e-12,
        Cj => 164e-15,
        Rleft => 50.0,
        Rright => 50.0
    )
    return circ, defs
end

# メイン処理：並列シミュレーション＆CSV 出力
function main()
    # 回路構築
    circuit, circuitdefs = build_circuit()

    # 周波数・ポンプ・ソース設定
    ws = 2π .* (1.0:0.01:16.0) .* 1e9
    wp = (2π * 8.7e9,)
    Ip = 3.6001e-6
    sources = [(mode=(1,), port=1, current=Ip)]
    Npump = (20,)
    Nmod = (10,)

    # 周波数チャンク分割
    nwrk = nworkers()
    chunks = [ws[i:nwrk:end] for i in 1:nwrk]

    # 並列実行
    @time results = pmap(
        chunk -> simulate_chunk(chunk, wp, sources, Nmod, Npump, circuit, circuitdefs),
        chunks
    )

    # 結果集約（collect で Vector に変換してから vcat）
    freqs = vcat([collect(r[1] ./ (2π * 1e9)) for r in results]...)
    gains = vcat([collect(10 .* log10.(abs2.(r[2].linearized.S((0,),2,(0,),1,:)))) for r in results]...)
    idx = sortperm(freqs)
    freq_vec = freqs[idx]
    gain_vec = gains[idx]

    # CSV 出力
    open("freq_gain_sim.csv", "w") do io
        println(io, "frequency_GHz,gain_dB")
        for (f, g) in zip(freq_vec, gain_vec)
            println(io, "$f,$g")
        end
    end
    println("Simulation & CSV output completed.")
end

# プロット関数
function plot_gain()
    data = readdlm("freq_gain_sim.csv", ',', skipstart=1)
    x, y = vec(data[:,1]), vec(data[:,2])
    if length(x) != length(y)
        error("xとyの長さが異なります: ", length(x), " ≠ ", length(y))
    end
    plt = plot(
        x, y;
        xlabel = "Frequency [GHz]",
        ylabel = "Gain [dB]",
        legend = false,
        seriestype = :line,
        marker = :none,
        line = (:solid, 2)
    )
    display(plt)
    savefig(plt, "gain.png")
end

# スクリプト直接実行時のエントリーポイント
if abspath(PROGRAM_FILE) == @__FILE__
    main()
    plot_gain()
end
