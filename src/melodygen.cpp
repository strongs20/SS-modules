#include "plugin.hpp"
#include "scale.hpp"
#include <random>
#include <string>

struct Melodygen : Module
{
	enum ParamId
	{
		RANGE_PARAM,
		SCALE_PARAM,
		KEY_PARAM,
		DISJUNCT_PARAM,
		NEWNOTEPROB_PARAM,
		GLIDEPROB_PARAM,
		PASSINGTONES_PARAM,
		REPETITIVE_PARAM,
		GLIDEAMOUNT_PARAM,
		PARAMS_LEN
	};
	enum InputId
	{
		GATE_INPUT,
		INPUTS_LEN
	};
	enum OutputId
	{
		GATE_OUTPUT,
		CV_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId
	{
		BLINK_LIGHT,
		LIGHTS_LEN
	};

	Melodygen()
	{
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(KEY_PARAM, 0.0, 11.0, 0.0, "Key");
		configParam(SCALE_PARAM, 0.0, ScaleUtils::NUM_SCALES - 1, 0, "Scale");

		configParam(DISJUNCT_PARAM, 0.f, 1.f, 0.5, "Disjunction");
		configParam(RANGE_PARAM, 1, 5, 0.f, "Octave Range"); // Snapping to integer values between 1 and 5

		configParam(NEWNOTEPROB_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GLIDEPROB_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PASSINGTONES_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REPETITIVE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GLIDEAMOUNT_PARAM, 0.f, 1.f, 0.f, "");

		configInput(GATE_INPUT, "");
		configOutput(GATE_OUTPUT, "");
		configOutput(CV_OUTPUT, "");
	}

	float lastCV = 0.f;
	int lastRandIndex = 0;
	bool gateOn = false;

	void process(const ProcessArgs &args) override
	{
		float gateIn = inputs[GATE_INPUT].getVoltage();

		// Read the scale knob and key knobs and determine the selected scale
		int scaleIndex = static_cast<int>(params[SCALE_PARAM].getValue());
		int keyValue = static_cast<int>(params[KEY_PARAM].getValue());
		int *selectedScale = ScaleUtils::SCALE_ARRAY[scaleIndex];
		int scaleSize = ScaleUtils::SCALE_SIZE[scaleIndex];

		// Get disjunct param
		float disjunctValue = static_cast<float>(params[DISJUNCT_PARAM].getValue());

		// Get the range in octaves from the knob
		int range = static_cast<int>(params[RANGE_PARAM].getValue());

		// Total number of notes across all octaves
		int totalNotes = range * scaleSize;

		// Check for positive edge of gate
		if (gateIn >= 10.f && !gateOn)
		{
			outputs[GATE_OUTPUT].setVoltage(10.f);
			std::random_device rd;
			std::mt19937 gen(rd());

			std::uniform_int_distribution<> dis;
			int randomNoteIndex;

			if (disjunctValue != 1.f)
			{
				std::vector<int> adjacentNotes;

				// Check lower adjacent note
				int notesBelowWeighted = static_cast<int>(static_cast<float>(lastRandIndex) * disjunctValue);
				int notesAboveWeighted = static_cast<int>(static_cast<float>(totalNotes - lastRandIndex - 1) * disjunctValue);

				int lowerBound = fmax(lastRandIndex - notesBelowWeighted, 0);
				// Check upper adjacent note
				int upperBound = fmin(lastRandIndex + notesAboveWeighted, totalNotes - 1);

				// Choose from adjacentNotes
				dis = std::uniform_int_distribution<>(lowerBound, upperBound);
				randomNoteIndex = dis(gen);
			}
			else
			{
				dis = std::uniform_int_distribution<>(0, totalNotes - 1);
				randomNoteIndex = dis(gen);
			}

			int randomOctave = randomNoteIndex / scaleSize;
			int randomNote = selectedScale[randomNoteIndex % scaleSize] + (12 * randomOctave) + keyValue;

			lastCV = (float)randomNote / 12.f; // Assuming 1V/Oct standard
			lastRandIndex = randomNoteIndex;
			gateOn = true;
		}
		else if (gateIn < 10.f && gateOn)
		{
			outputs[GATE_OUTPUT].setVoltage(10.f);
			gateOn = false;
		}
		else if (!gateOn)
		{
			outputs[GATE_OUTPUT].setVoltage(0.f);
			gateOn = false;
		}

		outputs[CV_OUTPUT].setVoltage(lastCV);
	}
};

struct MelodygenWidget : ModuleWidget
{
	std::shared_ptr<Font> font;

	MelodygenWidget(Melodygen *module)
	{
		setModule(module);
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/mfont.ttf"));
		setPanel(createPanel(asset::plugin(pluginInstance, "res/melodygen.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		ParamWidget *rangeKnob = createParamCentered<RoundBlackKnob>(mm2px(Vec(15, 32.299953)), module, Melodygen::RANGE_PARAM);
		if (rangeKnob->getParamQuantity())
		{
			rangeKnob->getParamQuantity()->snapEnabled = true;
		}
		addParam(rangeKnob);

		ParamWidget *scaleKnob = createParamCentered<RoundBlackKnob>(mm2px(Vec(38.099998, 32.299953)), module, Melodygen::SCALE_PARAM);
		if (scaleKnob->getParamQuantity())
		{
			scaleKnob->getParamQuantity()->snapEnabled = true;
		}
		addParam(scaleKnob);

		ParamWidget *keyKnob = createParamCentered<RoundBlackKnob>(mm2px(Vec(61.200001, 32.299953)), module, Melodygen::KEY_PARAM);
		if (keyKnob->getParamQuantity())
		{
			keyKnob->getParamQuantity()->snapEnabled = true;
		}
		addParam(keyKnob);

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15, 60.299976)), module, Melodygen::DISJUNCT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.099998, 60.299976)), module, Melodygen::NEWNOTEPROB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.200001, 60.299976)), module, Melodygen::GLIDEPROB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15, 88.29998)), module, Melodygen::PASSINGTONES_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.099998, 88.29998)), module, Melodygen::REPETITIVE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.200001, 88.29998)), module, Melodygen::GLIDEAMOUNT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15, 115.15134)), module, Melodygen::GATE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(46.920708, 115.15134)), module, Melodygen::GATE_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(63.891338, 115.15134)), module, Melodygen::CV_OUTPUT));
	}
	void draw(const DrawArgs &args) override
	{
		// Draw the original content
		ModuleWidget::draw(args);

		// Load font from cache
		std::string fontPath = asset::plugin(pluginInstance, "res/mfont.ttf");
		std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
		// Don't draw text if font failed to load
		if (font)
		{
			// Select font handle
			nvgFontFaceId(args.vg, font->handle);
			// Set font size and alignment
			nvgFontSize(args.vg, 10.0);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_BASELINE);
			nvgFillColor(args.vg, nvgRGBA(0, 0, 153, 153)); // RGBA: Blue

			// Convert the scale index to its string representation
			int scaleIndex = static_cast<int>(module->params[Melodygen::SCALE_PARAM].getValue());
			std::string scaleName = ScaleUtils::SCALE_NAMES[scaleIndex];

			// Convert the key index to its string representation
			int keyIndex = static_cast<int>(module->params[Melodygen::KEY_PARAM].getValue());
			std::string keyName = ScaleUtils::KEY_NAMES[keyIndex];

			// Draw the text at a position
			nvgText(args.vg, mm2px(38.099998), mm2px(41.5), scaleName.c_str(), NULL);
			nvgText(args.vg, mm2px(61.200001), mm2px(41.5), keyName.c_str(), NULL);
		}
	}
};

Model *melodygenModel = createModel<Melodygen, MelodygenWidget>("melodygen");