/* Pan Tilt Zoom device base object
 *
 * Copyright 2020 Grant Likely <grant.likely@secretlab.ca>
 *
 * SPDX-License-Identifier: GPLv2
 */
#pragma once

#include <memory>
#include <QDebug>
#include <QObject>
#include <QStringListModel>
#include <QtGlobal>
#include <obs.hpp>
#include <obs-frontend-api.h>

class PTZListModel : public QAbstractListModel {
	Q_OBJECT

public:
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	void do_reset() { beginResetModel(); endResetModel(); }
	Qt::ItemFlags flags(const QModelIndex &index) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
};

const QStringList default_preset_names({"Preset 1", "Preset 2", "Preset 3", "Preset 4", "Preset 5",
					"Preset 6", "Preset 7", "Preset 8", "Preset 9", "Preset 10"});

class PTZDevice : public QObject {
	Q_OBJECT

private:
	static PTZListModel ptz_list_model;
	static QVector<PTZDevice *> devices;

protected:
	std::string type;
	QStringListModel preset_names_model;

public:
	PTZDevice(std::string type) : QObject(), type(type)
	{
		devices.push_back(this);
		ptz_list_model.do_reset();
		preset_names_model.setStringList(default_preset_names);
	};
	~PTZDevice()
	{
		int row = devices.indexOf(this);
		if (row < 0)
			return;
		devices.remove(row);
		ptz_list_model.do_reset();
	};

	static PTZDevice* make_device(OBSData config);
	static PTZDevice* get_device(unsigned int index) { return devices.at(index); }
	static PTZDevice* get_device_by_name(QString &name);
	static unsigned int device_count() { return devices.size(); }
	static void delete_all() {
		while (!devices.empty())
			delete devices.first();
	}

	virtual void pantilt(double pan, double tilt) { Q_UNUSED(pan); Q_UNUSED(tilt); }
	virtual void pantilt_rel(int pan, int tilt) { Q_UNUSED(pan); Q_UNUSED(tilt); }
	virtual void pantilt_stop() { }
	virtual void pantilt_home() { }
	virtual void zoom_stop() { }
	virtual void zoom_tele() { }
	virtual void zoom_wide() { }
	virtual void memory_set(int i) { Q_UNUSED(i); }
	virtual void memory_recall(int i) { Q_UNUSED(i); }
	virtual void memory_reset(int i) { Q_UNUSED(i); }
	static QAbstractListModel * model() { return &ptz_list_model; }
	virtual QAbstractListModel * presetModel() { return &preset_names_model; }

	virtual void set_config(OBSData ptz_config);
	virtual OBSData get_config();

	virtual void setObjectName(QString name) {
		name = name.simplified();
		if (name == objectName())
			return;
		if (name == "")
			name = "Unnamed Device";
		QString new_name = name;
		for (int i = 1;; i++) {
			PTZDevice *ptz = get_device_by_name(new_name);
			if (!ptz)
				break;
			new_name = name + " " + QString::number(i);
			qDebug() << "new values" << new_name << name << i;
		}
		QObject::setObjectName(new_name);
	}
};

class PTZSimulator : public PTZDevice {
	Q_OBJECT

public:
	PTZSimulator() : PTZDevice("sim") { };
	PTZSimulator(OBSData config) : PTZDevice("sim") { set_config(config); };

	void pantilt(double pan, double tilt) { qDebug() << __func__ << "Pan" << pan << "Tilt" << tilt; }
	void pantilt_stop() { qDebug() << __func__; }
	void pantilt_home() { qDebug() << __func__; }
	void zoom_stop() { qDebug() << __func__; }
	void zoom_tele() { qDebug() << __func__; }
	void zoom_wide() { qDebug() << __func__; }
};
