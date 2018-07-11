// SPDX-License-Identifier: GPL-2.0
/*
 * ASoC driver for Boneblack Audio Extension
 *
 * Copyright (C) 2018 Christopher Obbard <chris@64studio.com>
 */

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>

#include "../codecs/adau17x1.h"
#include "../davinci/davinci-mcasp.h"

struct boneblack_audio_extension_drvdata {
	struct clk *mclk;
	unsigned sysclk;
};

static const unsigned int boneblack_audio_extension_rates[] = {
	8000, 12000, 16000, 24000, 32000, 48000, 96000
};

static struct snd_pcm_hw_constraint_list boneblack_audio_extension_rates_constraint = {
	.list = boneblack_audio_extension_rates,
	.count = ARRAY_SIZE(boneblack_audio_extension_rates),
};

static int boneblack_audio_extension_startup(struct snd_pcm_substream *substream)
{
	/* printk("AUDIOTEST %s entering function\n", __func__); */

	/* set sample rate constraints */
	snd_pcm_hw_constraint_list(substream->runtime, 0,
		SNDRV_PCM_HW_PARAM_RATE, &boneblack_audio_extension_rates_constraint);

	return 0;
}

static void boneblack_audio_extension_shutdown(struct snd_pcm_substream *substream)
{
	/* printk("AUDIOTEST %s entering function\n", __func__); */
}


static int boneblack_audio_extension_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *soc_card = rtd->card;
	int ret = 0, pll_rate = 0;
	unsigned sysclk = ((struct boneblack_audio_extension_drvdata *)
				snd_soc_card_get_drvdata(soc_card))->sysclk;

	/* printk("AUDIOTEST %s entering function\n", __func__); */
	/* printk("AUDIOTEST %s sample rate %u", __func__, params_rate(params)); */
	/* printk("AUDIOTEST %s sysclk %u", __func__, sysclk); */


	// set codec tdm slots
	snd_soc_dai_set_tdm_slot(rtd->codec_dais[0], 0x03, 0x03, 8, 32);
	snd_soc_dai_set_tdm_slot(rtd->codec_dais[1], 0x0c, 0x0c, 8, 32);
	snd_soc_dai_set_tdm_slot(rtd->codec_dais[2], 0x30, 0x30, 8, 32);

	// calculate frequency for codec pll
	switch (params_rate(params)) {
	case 48000:
	case 8000:
	case 12000:
	case 16000:
	case 24000:
	case 32000:
	case 96000:
		pll_rate = 48000 * 1024;
		break;
	case 44100:
	case 7350:
	case 11025:
	case 14700:
	case 22050:
	case 29400:
	case 88200:
		pll_rate = 44100 * 1024;
		break;
	default:
		return -EINVAL;
	}

	// setup the codec pll
	snd_soc_dai_set_pll(rtd->codec_dais[0], ADAU17X1_PLL, ADAU17X1_PLL_SRC_MCLK, sysclk, pll_rate);
	snd_soc_dai_set_sysclk(rtd->codec_dais[0], ADAU17X1_CLK_SRC_PLL, pll_rate, SND_SOC_CLOCK_IN);

	snd_soc_dai_set_pll(rtd->codec_dais[1], ADAU17X1_PLL, ADAU17X1_PLL_SRC_MCLK, sysclk, pll_rate);
	snd_soc_dai_set_sysclk(rtd->codec_dais[1], ADAU17X1_CLK_SRC_PLL, pll_rate, SND_SOC_CLOCK_IN);

	snd_soc_dai_set_pll(rtd->codec_dais[2], ADAU17X1_PLL, ADAU17X1_PLL_SRC_MCLK, sysclk, pll_rate);
	snd_soc_dai_set_sysclk(rtd->codec_dais[2], ADAU17X1_CLK_SRC_PLL, pll_rate, SND_SOC_CLOCK_IN);

	return ret;
}

static const struct snd_soc_ops boneblack_audio_extension_ops = {
	.startup = boneblack_audio_extension_startup,
	.shutdown = boneblack_audio_extension_shutdown,
	.hw_params = boneblack_audio_extension_hw_params,
};


static const struct snd_soc_dapm_widget boneblack_audio_extension_dapm_widgets[] = {
	/* routes for card 0 */
	SND_SOC_DAPM_LINE("C0 In 1", NULL),
	SND_SOC_DAPM_LINE("C0 In 2", NULL),
	SND_SOC_DAPM_LINE("C0 In 3-4", NULL),

	SND_SOC_DAPM_LINE("C0 Diff Out L", NULL),
	SND_SOC_DAPM_LINE("C0 Diff Out R", NULL),
	SND_SOC_DAPM_LINE("C0 Stereo Out", NULL),

	SND_SOC_DAPM_HP("C0 Capless HP Out", NULL),


	/* routes for card 1 */
	SND_SOC_DAPM_LINE("C1 In 1", NULL),
	SND_SOC_DAPM_LINE("C1 In 2", NULL),
	SND_SOC_DAPM_LINE("C1 In 3-4", NULL),

	SND_SOC_DAPM_LINE("C1 Diff Out L", NULL),
	SND_SOC_DAPM_LINE("C1 Diff Out R", NULL),
	SND_SOC_DAPM_LINE("C1 Stereo Out", NULL),

	SND_SOC_DAPM_HP("C1 Capless HP Out", NULL),


	/* routes for card 2 */
	SND_SOC_DAPM_LINE("C2 In 1", NULL),
	SND_SOC_DAPM_LINE("C2 In 2", NULL),
	SND_SOC_DAPM_LINE("C2 In 3-4", NULL),

	SND_SOC_DAPM_LINE("C2 Diff Out L", NULL),
	SND_SOC_DAPM_LINE("C2 Diff Out R", NULL),
	SND_SOC_DAPM_LINE("C2 Stereo Out", NULL),

	SND_SOC_DAPM_HP("C2 Capless HP Out", NULL),
};

static const struct snd_soc_dapm_route boneblack_audio_extension_audio_map[] = {
	/* routes for card 0 */
	{ "C0 LINP", NULL, "C0 In 1" },
	{ "C0 LINN", NULL, "C0 In 1"},
	{ "C0 RINP", NULL, "C0 In 2" },
	{ "C0 RINN", NULL, "C0 In 2" },
	{ "C0 LAUX", NULL, "C0 In 3-4" },
	{ "C0 RAUX", NULL, "C0 In 3-4" },

	{ "C0 In 1", NULL, "C0 MICBIAS" },
	{ "C0 In 2", NULL, "C0 MICBIAS" },

	{ "C0 Capless HP Out", NULL, "C0 LHP" },
	{ "C0 Capless HP Out", NULL, "C0 RHP" },

	{ "C0 Diff Out L", NULL, "C0 LOUT" },
	{ "C0 Diff Out R", NULL, "C0 ROUT" },
	{ "C0 Stereo Out", NULL, "C0 LOUT" },
	{ "C0 Stereo Out", NULL, "C0 ROUT" },

	/* routes for card 1 */
	{ "C1 LINP", NULL, "C1 In 1" },
	{ "C1 LINN", NULL, "C1 In 1"},
	{ "C1 RINP", NULL, "C1 In 2" },
	{ "C1 RINN", NULL, "C1 In 2" },
	{ "C1 LAUX", NULL, "C1 In 3-4" },
	{ "C1 RAUX", NULL, "C1 In 3-4" },

	{ "C1 In 1", NULL, "C1 MICBIAS" },
	{ "C1 In 2", NULL, "C1 MICBIAS" },

	{ "C1 Capless HP Out", NULL, "C1 LHP" },
	{ "C1 Capless HP Out", NULL, "C1 RHP" },

	{ "C1 Diff Out L", NULL, "C1 LOUT" },
	{ "C1 Diff Out R", NULL, "C1 ROUT" },
	{ "C1 Stereo Out", NULL, "C1 LOUT" },
	{ "C1 Stereo Out", NULL, "C1 ROUT" },

	/* routes for card 2 */
	{ "C2 LINP", NULL, "C2 In 1" },
	{ "C2 LINN", NULL, "C2 In 1"},
	{ "C2 RINP", NULL, "C2 In 2" },
	{ "C2 RINN", NULL, "C2 In 2" },
	{ "C2 LAUX", NULL, "C2 In 3-4" },
	{ "C2 RAUX", NULL, "C2 In 3-4" },

	{ "C2 In 1", NULL, "C2 MICBIAS" },
	{ "C2 In 2", NULL, "C2 MICBIAS" },

	{ "C2 Capless HP Out", NULL, "C2 LHP" },
	{ "C2 Capless HP Out", NULL, "C2 RHP" },

	{ "C2 Diff Out L", NULL, "C2 LOUT" },
	{ "C2 Diff Out R", NULL, "C2 ROUT" },
	{ "C2 Stereo Out", NULL, "C2 LOUT" },
	{ "C2 Stereo Out", NULL, "C2 ROUT" },
};

static int boneblack_audio_extension_dai_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_card *soc_card = rtd->card;

	struct boneblack_audio_extension_drvdata *drvdata =
		snd_soc_card_get_drvdata(soc_card);

	unsigned sysclk = drvdata->sysclk;

	int pll_rate = 0;

	/* printk("AUDIOTEST %s entering function\n", __func__); */

	// setup McASP
	snd_soc_dai_set_sysclk(cpu_dai, 0, sysclk, SND_SOC_CLOCK_IN);
	snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_DSP_A | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	snd_soc_dai_set_tdm_slot(cpu_dai, 0xFF, 0xFF, 8, 32);

	// lock codec PLL to sysclk
	// (we do not know what rate we want yet, so choose something that makes sense)
	pll_rate = 48000 * 1024;
	snd_soc_dai_set_pll(rtd->codec_dais[0], ADAU17X1_PLL, ADAU17X1_PLL_SRC_MCLK, sysclk, pll_rate);
	snd_soc_dai_set_pll(rtd->codec_dais[1], ADAU17X1_PLL, ADAU17X1_PLL_SRC_MCLK, sysclk, pll_rate);
	snd_soc_dai_set_pll(rtd->codec_dais[2], ADAU17X1_PLL, ADAU17X1_PLL_SRC_MCLK, sysclk, pll_rate);

	// enable codec clock
	snd_soc_dai_set_sysclk(rtd->codec_dais[0], ADAU17X1_CLK_SRC_PLL, pll_rate, SND_SOC_CLOCK_IN);
	snd_soc_dai_set_sysclk(rtd->codec_dais[1], ADAU17X1_CLK_SRC_PLL, pll_rate, SND_SOC_CLOCK_IN);
	snd_soc_dai_set_sysclk(rtd->codec_dais[2], ADAU17X1_CLK_SRC_PLL, pll_rate, SND_SOC_CLOCK_IN);

	// set codec format (this is not the same as cpu format due to possible bug in codec hardware)
	snd_soc_dai_set_fmt(rtd->codec_dais[0], SND_SOC_DAIFMT_DSP_A | SND_SOC_DAIFMT_IB_IF | SND_SOC_DAIFMT_CBS_CFS);
	snd_soc_dai_set_fmt(rtd->codec_dais[1], SND_SOC_DAIFMT_DSP_A | SND_SOC_DAIFMT_IB_IF | SND_SOC_DAIFMT_CBS_CFS);
	snd_soc_dai_set_fmt(rtd->codec_dais[2], SND_SOC_DAIFMT_DSP_A | SND_SOC_DAIFMT_IB_IF | SND_SOC_DAIFMT_CBS_CFS);

	return 0;
}

static struct snd_soc_dai_link_component boneblack_audio_extension_codec_components[] = {
	{
		.name = "adau1761.2-0038",
		.dai_name = "adau-hifi",
	},
	{
		.name = "adau1761.2-0039",
		.dai_name = "adau-hifi",
	},
	{
		.name = "adau1761.2-003a",
		.dai_name = "adau-hifi",
	},
};

static struct snd_soc_codec_conf boneblack_audio_extension_codec_conf[] = {
	{
		.dev_name = "adau1761.2-0038",
		.name_prefix = "C0",
	},
	{
		.dev_name = "adau1761.2-0039",
		.name_prefix = "C1",
	},
	{
		.dev_name = "adau1761.2-003a",
		.name_prefix = "C2",
	},
};


static struct snd_soc_dai_link boneblack_audio_extension_dailink = {
	.name = "Boneblack Audio Extension",
	.stream_name = "Boneblack Audio Extension",

	.codecs = boneblack_audio_extension_codec_components,
	.num_codecs = ARRAY_SIZE(boneblack_audio_extension_codec_components),

	.ops = &boneblack_audio_extension_ops,
	.init = boneblack_audio_extension_dai_init,

};


static struct boneblack_audio_extension_drvdata boneblack_audio_extension_drvdata_clkinfo = {
	.sysclk = 0,
	.mclk = NULL,
};

static struct snd_soc_card boneblack_audio_extension_soc_card = {
	.name = "boneaudioext",
	.owner = THIS_MODULE,
	.dai_link = &boneblack_audio_extension_dailink,
	.num_links = 1,
	.dapm_widgets = boneblack_audio_extension_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(boneblack_audio_extension_dapm_widgets),
	.dapm_routes = boneblack_audio_extension_audio_map,
	.num_dapm_routes = ARRAY_SIZE(boneblack_audio_extension_audio_map),
	.drvdata = &boneblack_audio_extension_drvdata_clkinfo,

	.codec_conf = boneblack_audio_extension_codec_conf,
	.num_configs = ARRAY_SIZE(boneblack_audio_extension_codec_conf),
};

static int boneblack_audio_extension_dt_init(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *cpu_np;
	struct snd_soc_dai_link *dailink = &boneblack_audio_extension_dailink;
	struct boneblack_audio_extension_drvdata *drvdata;
	struct clk *mclk = NULL;
	int ret;

	/* printk("AUDIOTEST %s entering function\n", __func__); */

	if (!np) {
		dev_err(&pdev->dev, "only device tree supported\n");
		return -EINVAL;
	}

	/* load cpu dai from devicetree */
	cpu_np = of_parse_phandle(np, "boneblack-audio-extension,mcasp", 0);
	if (!cpu_np) {
		dev_err(&pdev->dev, "failed to get boneblack-audio-extension,mcasp from DTS\n");
		ret = -EINVAL;
		return ret;
	}
	dailink->cpu_of_node = cpu_np;
	dailink->platform_of_node = cpu_np;
	of_node_put(cpu_np);

	/* load codec dai from devicetree */
	/*codec_np = of_parse_phandle(np, "boneblack-audio-extension,codec0", 0);
	if (!codec_np) {
		dev_err(&pdev->dev, "failed to get boneblack-audio-extension,codec0 from DTS\n");
		ret = -EINVAL;
		return ret;
	}
	dailink->codec_of_node = codec_np;
	of_node_put(codec_np);*/

	drvdata = devm_kzalloc(&pdev->dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	/* load clock from dts */
	mclk = devm_get_clk_from_child(&pdev->dev, np, NULL);
	if (!IS_ERR(mclk)) {
		drvdata->sysclk = clk_get_rate(mclk);
		drvdata->mclk = mclk;

		/* printk("AUDIOTEST %s sysclk: %u\n", __func__, drvdata->sysclk); */
	}

	snd_soc_card_set_drvdata(&boneblack_audio_extension_soc_card, drvdata);

	return 0;
}

static int boneblack_audio_extension_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &boneblack_audio_extension_soc_card;
	struct boneblack_audio_extension_drvdata *drvdata = NULL;
	int ret;

	/* printk("AUDIOTEST %s entering function\n", __func__); */

	/* read devicetree */
	card->dev = &pdev->dev;
	ret = boneblack_audio_extension_dt_init(pdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to init dt info\n");
		return ret;
	}

	drvdata = snd_soc_card_get_drvdata(card);

	/* enable mclk */
	if (drvdata->mclk) {
		ret = clk_prepare_enable(drvdata->mclk);
		if (ret) {
			dev_err(&pdev->dev, "failed to enable clk\n");
			return ret;
		}
	}

	/* register cards */
	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed\n");
		return ret;
	}

	return 0;
}

static int boneblack_audio_extension_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct boneblack_audio_extension_drvdata *drvdata =
		snd_soc_card_get_drvdata(card);

	/* printk("AUDIOTEST %s entering function\n", __func__); */

	/* disable clock */
	if (drvdata->mclk) {
		printk("AUDIOTEST %s disabling clock\n", __func__);
		clk_disable_unprepare(drvdata->mclk);
	}

	snd_soc_unregister_card(card);

	return 0;
}

static const struct of_device_id boneblack_audio_extension_dt_ids[] = {
	{ .compatible = "64studio,boneblack-audio-extension", },
	{ }
};
MODULE_DEVICE_TABLE(of, boneblack_audio_extension_dt_ids);

static struct platform_driver boneblack_audio_extension_driver = {
	.driver = {
		.name = "boneblack-audio-extension",
		.pm = &snd_soc_pm_ops,
		.of_match_table = boneblack_audio_extension_dt_ids,
	},
	.probe = boneblack_audio_extension_probe,
	.remove = boneblack_audio_extension_remove,
};
module_platform_driver(boneblack_audio_extension_driver);

MODULE_ALIAS("platform:boneblack-audio-extension");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ASoC driver for Boneblack Audio Extension");
MODULE_AUTHOR("Christopher Obbard <chris@64studio.com>");
